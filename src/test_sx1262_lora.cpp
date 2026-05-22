/*
 * SX1262 LoRa Reception Test
 * Tests basic radio functionality before attempting FSK/WMBus
 */

#include <Arduino.h>
#include <RadioLib.h>

#include "hwconfig.h"

SPIClass* spi;
SX1262* radio;

volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;

// Interrupt handler
void IRAM_ATTR setFlag(void) {
    if (!enableInterrupt) {
        return;
    }
    receivedFlag = true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n\n=================================");
    Serial.println("SX1262 LoRa Reception Test");
    Serial.println("=================================\n");

    // Initialize SPI
    spi = new SPIClass(HSPI);
    spi->begin(SX1262_SCK, SX1262_MISO, SX1262_MOSI, SX1262_NSS);

    Serial.print("SPI initialized on pins: SCK=");
    Serial.print(SX1262_SCK);
    Serial.print(", MISO=");
    Serial.print(SX1262_MISO);
    Serial.print(", MOSI=");
    Serial.print(SX1262_MOSI);
    Serial.print(", NSS=");
    Serial.println(SX1262_NSS);

    // Create radio instance
    radio = new SX1262(new Module(SX1262_NSS, SX1262_DIO1, SX1262_RESET, SX1262_BUSY, *spi));

    Serial.println("\nInitializing SX1262 in LoRa mode...");

    // Initialize with default LoRa settings (868 MHz)
    int state = radio->begin(868.0,   // frequency in MHz
                             125.0,   // bandwidth in kHz
                             9,       // spreading factor
                             7,       // coding rate
                             0x12,    // sync word
                             10,      // output power in dBm
                             8,       // preamble length
                             0,       // TCXO voltage (0 = disabled)
                             false);  // use LDO

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("[OK] SX1262 initialized successfully!");
    } else {
        Serial.print("[ERROR] Failed to initialize SX1262, code: ");
        Serial.println(state);
        Serial.println("Trying without TCXO parameter...");

        state = radio->begin();
        if (state == RADIOLIB_ERR_NONE) {
            Serial.println("[OK] SX1262 initialized with default settings!");
        } else {
            Serial.print("[ERROR] Initialization failed, code: ");
            Serial.println(state);
            while (1);
        }
    }

    // Set frequency to 868 MHz (same as WMBus)
    state = radio->setFrequency(868.95);
    Serial.print("Set frequency to 868.95 MHz: ");
    Serial.println(state == RADIOLIB_ERR_NONE ? "OK" : "Failed");

    // Configure DIO2 as RF switch
    state = radio->setDio2AsRfSwitch(true);
    Serial.print("Configure DIO2 RF switch: ");
    Serial.println(state == RADIOLIB_ERR_NONE ? "OK" : "Failed");

    // Set interrupt handler
    radio->setDio1Action(setFlag);
    Serial.println("Interrupt handler attached to DIO1");

    // Start receiving
    state = radio->startReceive();
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("[OK] Started receiving!");
    } else {
        Serial.print("[ERROR] Failed to start receiver, code: ");
        Serial.println(state);
    }

    Serial.println("\n=================================");
    Serial.println("Waiting for LoRa packets...");
    Serial.println("(This won't receive WMBus - just testing radio HW)");
    Serial.println("=================================\n");
}

int packetCount = 0;
unsigned long lastStatus = 0;

void loop() {
    // Check if packet was received
    if (receivedFlag) {
        enableInterrupt = false;
        receivedFlag = false;

        Serial.println("\n[!] INTERRUPT DETECTED!");

        // Read packet
        uint8_t buffer[256];
        int state = radio->readData(buffer, 256);

        if (state == RADIOLIB_ERR_NONE) {
            packetCount++;

            Serial.print("[PACKET #");
            Serial.print(packetCount);
            Serial.println("] Received successfully!");

            // Print packet info
            Serial.print("  Length: ");
            Serial.print(radio->getPacketLength());
            Serial.println(" bytes");

            Serial.print("  RSSI: ");
            Serial.print(radio->getRSSI());
            Serial.println(" dBm");

            Serial.print("  SNR: ");
            Serial.print(radio->getSNR());
            Serial.println(" dB");

            Serial.print("  Frequency Error: ");
            Serial.print(radio->getFrequencyError());
            Serial.println(" Hz");

            // Print first 16 bytes of data
            Serial.print("  Data (hex): ");
            int len = radio->getPacketLength();
            for (int i = 0; i < len && i < 16; i++) {
                if (buffer[i] < 0x10)
                    Serial.print("0");
                Serial.print(buffer[i], HEX);
                Serial.print(" ");
            }
            if (len > 16) {
                Serial.print("... (");
                Serial.print(len - 16);
                Serial.print(" more bytes)");
            }
            Serial.println();

        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            Serial.println("  [CRC Error] Packet corrupted");
        } else {
            Serial.print("  [ERROR] Failed to read data, code: ");
            Serial.println(state);
        }

        // Restart receiver
        radio->startReceive();
        enableInterrupt = true;
    }

    // Print status every 10 seconds
    if (millis() - lastStatus > 10000) {
        lastStatus = millis();
        Serial.print("[");
        Serial.print(millis() / 1000);
        Serial.print("s] Listening... ");
        Serial.print("Packets received: ");
        Serial.println(packetCount);
    }

    delay(10);
}
