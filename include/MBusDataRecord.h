#ifndef MBUS_DATA_RECORD_H
#define MBUS_DATA_RECORD_H

#include <Arduino.h>

// M-Bus Data Information Field (DIF) definitions
#define DIF_NO_DATA     0x00
#define DIF_8BIT_INT    0x01
#define DIF_16BIT_INT   0x02
#define DIF_24BIT_INT   0x03
#define DIF_32BIT_INT   0x04
#define DIF_32BIT_FLOAT 0x05
#define DIF_48BIT_INT   0x06
#define DIF_64BIT_INT   0x07

// M-Bus Value Information Field (VIF) definitions
#define VIF_ENERGY_WH   0x00  // 0x00-0x07 Energy Wh (10^(n-3) Wh)
#define VIF_ENERGY_J    0x08  // 0x08-0x0F Energy J (10^(n) J)
#define VIF_VOLUME      0x10  // 0x10-0x17 Volume m³ (10^(n-6) m³)
#define VIF_MASS        0x18  // 0x18-0x1F Mass kg (10^(n-3) kg)
#define VIF_VOLUME_FLOW 0x38  // 0x38-0x3F Volume flow m³/h (10^(n-6) m³/h)
#define VIF_POWER_W     0x28  // 0x28-0x2F Power W (10^(n-3) W)
#define VIF_TEMP        0x58  // 0x58-0x5F Temperature °C (10^(n-3) °C)
#define VIF_DATE        0x6C  // Date type G
#define VIF_DATETIME    0x6D  // Date and time type F

struct MBusRecord {
    uint8_t dif;
    uint8_t vif;
    union {
        uint32_t intValue;
        float floatValue;
        uint16_t dateValue;
    };
    int dataLength;
};

class MBusDecoder {
public:
    static int parseRecord(const uint8_t* data, int offset, int maxLen, MBusRecord& record);
    static void printRecord(const MBusRecord& record);
    static int getDataLength(uint8_t dif);
    static float applyMultiplier(uint8_t vif, uint32_t rawValue);
};

#endif
