/*
 Copyright (C) 2020 chester4444@wolke7.net
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Uncoment the line below for exprimental FlowIQ 2200 support. NOTE: It only reports the volume, other values like temperature will be 0.
 */

// #define FLOWIQ2200

#include "WMbusFrame.h"

#include "MBusDataRecord.h"
#include "debug.h"

void mqttMyData(const char* debug_str);
void mqttMyDataJson(const char* debug_str);

WMBusFrame::WMBusFrame() {
    aes128.setKey(key, sizeof(key));
}

void WMBusFrame::check() {
    // check meterId
    DEBUG_PRINTF("MeterId check: Config=[%02X %02X %02X %02X] Frame=[%02X %02X %02X %02X] ", meterId[0], meterId[1], meterId[2], meterId[3],
                 payload[6], payload[5], payload[4], payload[3]);

    for (uint8_t i = 0; i < 4; i++) {
        if (meterId[i] != payload[6 - i]) {
            DEBUG_PRINTF("MISMATCH\n\r");
            isValid = false;
            return;
        }
    }

    DEBUG_PRINTF("Payload: ");
    for (int k = 0; k < length; k++) {
        DEBUG_PRINTF("%02x", payload[k]);
    }
    DEBUG_PRINTF("\n\r");

    isValid = true;
}

void WMBusFrame::printMeterInfo(uint8_t* data, size_t len) {
    // init positions for compact frame
    int pos_tt = 9;   // total consumption, 9, 10, 11, 12
    int pos_tg = 13;  // target consumption 13, 14, 15, 16
    // int pos_ic = 7; // info codes
    int pos_ft = 17;  // flow temp
    int pos_at = 18;  // ambient temp

    uint32_t tt = 0;  // total volume
    uint32_t tg = 0;  // target volume

    char mqttstring[25];
    char mqttjsondstring[100];

    DEBUG_PRINTF("Data: ");
    for (int k = 0; k < len; k++) {
        DEBUG_PRINTF("%02x", data[k]);
    }
    DEBUG_PRINTF("\n\r");
    DEBUG_PRINTF("Frame type byte [2]: 0x%02x\n\r", data[2]);

    // Decode format signature and data CRC for compact frames
    if (data[2] == 0x79) {
        uint16_t formatSig = data[4] | (data[5] << 8);
        uint16_t dataCrc = data[6] | (data[7] << 8);
        DEBUG_PRINTF("Format signature [4-5]: 0x%04x\n\r", formatSig);
        DEBUG_PRINTF("Data CRC [6-7]: 0x%04x\n\r", dataCrc);
    } else if (data[2] == 0x78) {
        // Status byte only meaningful for long frames
        uint8_t status = data[3];
        DEBUG_PRINTF("Status [3]: 0x%02x - ", status);
        if (status == 0x00) {
            DEBUG_PRINTF("OK");
        } else {
            if (status & 0x01)
                DEBUG_PRINTF("APP_BUSY ");
            if (status & 0x02)
                DEBUG_PRINTF("APP_PERM_ERR ");
            if (status & 0x04)
                DEBUG_PRINTF("APP_TEMP_ERR ");
            if (status & 0x08)
                DEBUG_PRINTF("APP_LOW_BATTERY ");
            if (status & 0x10)
                DEBUG_PRINTF("MANUFACTURER_SPEC ");
            if (status & 0x20)
                DEBUG_PRINTF("MANUFACTURER_SPEC ");
            if (status & 0x40)
                DEBUG_PRINTF("MANUFACTURER_SPEC ");
            if (status & 0x80)
                DEBUG_PRINTF("MANUFACTURER_SPEC ");
        }
        DEBUG_PRINTF("\n\r");
    }

#ifdef FLOWIQ2200
    if (data[2] == 0x79)  // compact frame
    {
        pos_tt = 29;
    }
#else
    // Multical 21
    if (data[2] == 0x79)  // compact frame
    {
        DEBUG_PRINTF("Detected: Compact frame (0x79) - using fixed positions\n\r");

        // Data fields at original positions (confirmed correct)
        pos_tt = 9;   // bytes 9-12: total volume
        pos_tg = 13;  // bytes 13-16: target volume
        pos_ft = 17;  // byte 17: flow temp
        pos_at = 18;  // byte 18: ambient temp

        // Note: byte 8 purpose unclear, may be part of format or status
        // Info codes location still TBD - not at fixed position in this format
    } else if (data[2] == 0x78)  // long frame
    {
        DEBUG_PRINTF("Detected: Long frame (0x78) - parsing M-Bus records\n\r");
        // Parse M-Bus data records starting after status byte
        int offset = 4;  // Skip CRC(2) + frame_type(1) + status(1)
        MBusRecord record;

        while (offset < len - 1) {
            int nextOffset = MBusDecoder::parseRecord(data, offset, len, record);
            if (nextOffset <= offset)
                break;

            MBusDecoder::printRecord(record);

            // Extract values based on VIF
            uint8_t vifBase = record.vif & 0xF8;
            if (vifBase == 0x10) {                  // Volume
                if ((record.dif & 0x0F) == 0x04) {  // 32-bit
                    if ((record.dif & 0x40) == 0) {
                        // Current volume
                        tt = record.intValue;
                        DEBUG_PRINTF("  → Current volume\n\r");
                    } else {
                        // Stored volume (target)
                        tg = record.intValue;
                        DEBUG_PRINTF("  → Target volume\n\r");
                    }
                }
            } else if (vifBase == 0x58) {  // Temperature
                // Temperatures are at end, will be read separately
            } else if (record.vif == 0x6C) {  // Date
                DEBUG_PRINTF("  → Date field\n\r");
            }

            offset = nextOffset;
        }

        // Temperatures are still at fixed positions at the end
        pos_ft = len - 4;  // 4 bytes from end
        pos_at = len - 1;  // Last byte

        DEBUG_PRINTF("Temps at positions: ft=%d at=%d\n\r", pos_ft, pos_at);
    }
#endif
    else {
        DEBUG_PRINTF("Unknown frame type: 0x%02x - rejecting\n\r", data[2]);
        isValid = false;
        return;
    }

    uint16_t calc_crc = crc16_EN13757(data + 2, len - 2);
    uint16_t read_crc = data[1] << 8 | data[0];
    DEBUG_PRINTF("calc_crc: 0x%04x\n\r", calc_crc);
    DEBUG_PRINTF("read_crc: 0x%04x\n\r", read_crc);

    if (calc_crc == read_crc) {
        DEBUG_PRINTF("CRC: OK\n\r");
    } else {
        DEBUG_PRINTF("CRC: ERROR - discarding frame\n\r");
        isValid = false;
        return;
    }

    // For compact frames (0x79), read from fixed positions
    if (data[2] == 0x79) {
        tt = data[pos_tt] + (data[pos_tt + 1] << 8) + (data[pos_tt + 2] << 16) + (data[pos_tt + 3] << 24);
        tg = data[pos_tg] + (data[pos_tg + 1] << 8) + (data[pos_tg + 2] << 16) + (data[pos_tg + 3] << 24);
    }
    // For long frames, tt and tg were already parsed from M-Bus records

    char total[10];
    snprintf(total, sizeof(total), "%d.%03d", tt / 1000, tt % 1000);
    DEBUG_PRINTF("total: %s m%c - ", total, 179);
    snprintf(mqttstring, sizeof(mqttstring), "%d.%03d", tt / 1000, tt % 1000);
    mqttMyData(mqttstring);

#ifdef FLOWIQ2200
    snprintf(mqttjsondstring, sizeof(mqttjsondstring),
             "{\"CurrentValue\": %d.%03d,\"MonthStartValue\": %d.%03d,\"WaterTemp\": %2d,\"RoomTemp\": %2d}", tt / 1000, tt % 1000, 0, 0, 0,
             0);
#else
    char target[10];
    snprintf(target, sizeof(target), "%d.%03d", tg / 1000, tg % 1000);
    DEBUG_PRINTF("target: %s m%c - ", target, 179);

    char flow_temp[3];
    snprintf(flow_temp, sizeof(flow_temp), "%2d", data[pos_ft]);
    DEBUG_PRINTF("%s %cC - ", flow_temp, 176);

    char ambient_temp[3];
    snprintf(ambient_temp, sizeof(ambient_temp), "%2d", data[pos_at]);
    DEBUG_PRINTF("%s %cC\n\r", ambient_temp, 176);

    snprintf(mqttjsondstring, sizeof(mqttjsondstring),
             "{\"CurrentValue\": %d.%03d,\"MonthStartValue\": %d.%03d,\"WaterTemp\": %2d,\"RoomTemp\": %2d}", tt / 1000, tt % 1000,
             tg / 1000, tg % 1000, data[pos_ft], data[pos_at]);

    // Store decoded values for display
    currentValue = tt / 1000.0f;
    monthStartValue = tg / 1000.0f;
    waterTemp = data[pos_ft];
    roomTemp = data[pos_at];
#endif
    mqttMyDataJson(mqttjsondstring);
}

void WMBusFrame::decode() {
    // check meterId, CRC
    check();
    if (!isValid)
        return;

    uint8_t cipherLength = length - 2 - 16;  // cipher starts at index 16, remove 2 crc bytes
    memcpy(cipher, &payload[16], cipherLength);

    memset(iv, 0, sizeof(iv));  // padding with 0
    memcpy(iv, &payload[1], 8);
    iv[8] = payload[10];
    memcpy(&iv[9], &payload[12], 4);

    aes128.setIV(iv, sizeof(iv));
    aes128.decrypt(plaintext, (const uint8_t*)cipher, cipherLength);

    DEBUG_PRINTF("C:     ");
    for (size_t i = 0; i < cipherLength; i++) {
        DEBUG_PRINTF("%02X", cipher[i]);
    }
    DEBUG_PRINTLN();
    DEBUG_PRINTF("P(%d): ", cipherLength);
    for (size_t i = 0; i < cipherLength; i++) {
        DEBUG_PRINTF("%02X", plaintext[i]);
    }
    DEBUG_PRINTLN();

    printMeterInfo(plaintext, cipherLength);
}

uint16_t WMBusFrame::crc16_EN13757(uint8_t* data, size_t len) {
    uint16_t crc = 0x0000;

    assert(len == 0 || data != NULL);

    for (size_t i = 0; i < len; ++i) {
        crc = crc16_EN13757_per_byte(crc, data[i]);
    }

    return (~crc);
}

#define CRC16_EN_13757 0x3D65

uint16_t WMBusFrame::crc16_EN13757_per_byte(uint16_t crc, uint8_t b) {
    unsigned char i;

    for (i = 0; i < 8; i++) {
        if (((crc & 0x8000) >> 8) ^ (b & 0x80)) {
            crc = (crc << 1) ^ CRC16_EN_13757;
        } else {
            crc = (crc << 1);
        }

        b <<= 1;
    }

    return crc;
}
