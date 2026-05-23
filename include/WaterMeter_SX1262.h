/*
 SX1262 implementation for WMBus Mode C1 reception
 Based on original CC1101 implementation by chester4444@wolke7.net

 This implementation uses RadioLib for SX1262 FSK communication
 to receive WMBus Mode C1 frames from Kamstrup Multical 21 meters.
*/

#ifndef _WATERMETER_SX1262_H_
#define _WATERMETER_SX1262_H_

#include <Arduino.h>
#include <RadioLib.h>

#include "WMbusFrame.h"
#include "hwconfig.h"

class WaterMeter {
private:
    SX1262* radio;
    SPIClass* spi;

    // Interrupt flag for packet reception
    static volatile bool packetReceived;

    // Interrupt handler (must be static)
    static void IRAM_ATTR onReceive(void);

    // Receive a WMBus frame
    void receive(WMBusFrame* frame);

public:
    // Constructor
    WaterMeter(void);

    // Destructor
    ~WaterMeter(void);

    // Initialize SX1262 for WMBus Mode C1 reception
    void begin();

    // Must be called frequently, returns true if a valid frame was received
    bool isFrameAvailable(void);

    // Get RSSI of last received packet (dBm)
    int16_t getRSSI(void);

    // Get SNR of last received packet (dB)
    float getSNR(void);
};

#endif  // _WATERMETER_SX1262_H_
