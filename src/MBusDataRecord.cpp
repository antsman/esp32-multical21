#include "MBusDataRecord.h"

#include "debug.h"

int MBusDecoder::getDataLength(uint8_t dif) {
    uint8_t dataField = dif & 0x0F;

    switch (dataField) {
        case 0x00:
            return 0;  // No data
        case 0x01:
            return 1;  // 8-bit integer
        case 0x02:
            return 2;  // 16-bit integer
        case 0x03:
            return 3;  // 24-bit integer
        case 0x04:
            return 4;  // 32-bit integer
        case 0x05:
            return 4;  // 32-bit float
        case 0x06:
            return 6;  // 48-bit integer
        case 0x07:
            return 8;  // 64-bit integer
        case 0x09:
            return 2;  // 2-digit BCD
        case 0x0A:
            return 4;  // 4-digit BCD
        case 0x0B:
            return 6;  // 6-digit BCD
        case 0x0C:
            return 8;  // 8-digit BCD
        case 0x0D:
            return 0;  // Variable length
        case 0x0E:
            return 6;  // 12-digit BCD
        case 0x0F:
            return 0;  // Special functions
        default:
            return 0;
    }
}

float MBusDecoder::applyMultiplier(uint8_t vif, uint32_t rawValue) {
    // Extract multiplier from VIF
    int8_t exponent = (vif & 0x07) - 3;  // Most VIFs use n-3 for exponent

    if ((vif & 0xF8) == 0x10) {         // Volume m³
        exponent = (vif & 0x07) - 6;    // Volume uses 10^(n-6)
    } else if ((vif & 0xF8) == 0x58) {  // Temperature °C
        exponent = (vif & 0x07) - 3;
    }

    float multiplier = 1.0;
    for (int i = 0; i < abs(exponent); i++) {
        multiplier *= 10.0;
    }

    if (exponent < 0) {
        return rawValue / multiplier;
    } else {
        return rawValue * multiplier;
    }
}

int MBusDecoder::parseRecord(const uint8_t* data, int offset, int maxLen, MBusRecord& record) {
    if (offset >= maxLen)
        return -1;

    record.dif = data[offset++];

    // Skip DIF extension bytes (if bit 7 is set)
    while ((record.dif & 0x80) && offset < maxLen) {
        offset++;
        record.dif = data[offset++];
    }

    if (offset >= maxLen)
        return -1;
    record.vif = data[offset++];

    // Skip VIF extension bytes (if bit 7 is set)
    while ((record.vif & 0x80) && offset < maxLen) {
        offset++;
        record.vif = data[offset++];
    }

    record.dataLength = getDataLength(record.dif);

    if (offset + record.dataLength > maxLen)
        return -1;

    // Read data value
    record.intValue = 0;
    uint8_t dataField = record.dif & 0x0F;

    // Check if BCD format
    if (dataField >= 0x09 && dataField <= 0x0E) {
        // BCD decode
        for (int i = 0; i < record.dataLength; i++) {
            uint8_t bcdByte = data[offset + i];
            uint8_t low = bcdByte & 0x0F;
            uint8_t high = (bcdByte >> 4) & 0x0F;
            record.intValue = record.intValue * 100 + high * 10 + low;
        }
    } else {
        // Binary format
        for (int i = 0; i < record.dataLength && i < 4; i++) {
            record.intValue |= ((uint32_t)data[offset + i]) << (8 * i);
        }
    }

    return offset + record.dataLength;
}

void MBusDecoder::printRecord(const MBusRecord& record) {
    DEBUG_PRINTF("  DIF=0x%02X VIF=0x%02X ", record.dif, record.vif);

    // Decode DIF function field
    uint8_t function = (record.dif >> 4) & 0x03;
    uint8_t storageNum = (record.dif >> 6) & 0x01;
    const char* functionStr = "";
    if (function == 0)
        functionStr = "Instant";
    else if (function == 1)
        functionStr = "Max";
    else if (function == 2)
        functionStr = "Min";
    else if (function == 3)
        functionStr = "Error";

    if (storageNum || function) {
        DEBUG_PRINTF("[%s", functionStr);
        if (storageNum)
            DEBUG_PRINTF(" Stor#%d", storageNum);
        DEBUG_PRINTF("] ");
    }

    // If this is an error value, highlight it
    if (function == 3) {
        DEBUG_PRINTF("⚠️ ERROR VALUE ⚠️ ");
    }

    uint8_t vifBase = record.vif & 0xF8;

    if (vifBase == 0x10) {  // Volume
        float volume = applyMultiplier(record.vif, record.intValue);
        DEBUG_PRINTF("Volume: %.3f m³", volume);
    } else if (vifBase == 0x58) {  // Temperature
        float temp = applyMultiplier(record.vif, record.intValue);
        DEBUG_PRINTF("Temperature: %.1f °C", temp);
    } else if (record.vif == 0x5B) {  // VIF 0x5B = External/Flow temperature
        DEBUG_PRINTF("Flow Temperature: %d °C", record.intValue);
    } else if (record.vif == 0x67) {  // VIF 0x67 = Ambient/Room temperature
        DEBUG_PRINTF("Ambient Temperature: %d °C", record.intValue);
    } else if (vifBase == 0x60) {  // 0x60-0x6F = Date/time types (excluding 0x67)
        if (record.vif == 0x6C) {
            DEBUG_PRINTF("Time Point (Date): raw=0x%04X", record.intValue);
        } else {
            DEBUG_PRINTF("Time/Date (VIF 0x%02X): raw=0x%04X", record.vif, record.intValue);
        }
    } else if (record.vif == 0x6C || record.vif == 0x6D) {  // Date/DateTime
        // M-Bus date format: Type G (16-bit)
        uint16_t dateVal = record.intValue & 0xFFFF;
        int day = dateVal & 0x1F;
        int month = (dateVal >> 5) & 0x0F;
        int year = ((dateVal >> 9) & 0x7F) + 2000;
        DEBUG_PRINTF("Date: %04d-%02d-%02d", year, month, day);
    } else if (vifBase == 0x38) {  // Volume flow
        float flow = applyMultiplier(record.vif, record.intValue);
        DEBUG_PRINTF("Flow: %.3f m³/h", flow);
    } else {
        DEBUG_PRINTF("VIF=0x%02X Value: %u (raw)", record.vif, record.intValue);
    }

    DEBUG_PRINTF("\n\r");
}
