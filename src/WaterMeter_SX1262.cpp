/*
 SX1262 implementation for WMBus Mode C1 reception
 Based on original CC1101 implementation by chester4444@wolke7.net

 WMBus Mode C1 Parameters:
 - Frequency: 868.95 MHz
 - Modulation: 2-FSK
 - Bit rate: 100 kbps
 - Frequency deviation: ±50 kHz
 - Sync word: 0x543D (preamble)
*/

#include "WaterMeter_SX1262.h"

#include "debug.h"

// Static member initialization
volatile bool WaterMeter::packetReceived = false;

// Interrupt handler for DIO1 (packet received)
void IRAM_ATTR WaterMeter::onReceive(void) {
    packetReceived = true;
}

// Constructor
WaterMeter::WaterMeter() {
    radio = nullptr;
    spi = nullptr;
}

// Destructor
WaterMeter::~WaterMeter() {
    if (radio != nullptr) {
        delete radio;
    }
}

// Initialize SX1262 for WMBus Mode C1 reception
void WaterMeter::begin() {
    DEBUG_PRINTLN("Initializing SX1262 for WMBus Mode C1...");

    // Create SPI instance for HSPI (SPI2)
    spi = new SPIClass(HSPI);
    spi->begin(SX1262_SCK, SX1262_MISO, SX1262_MOSI, SX1262_NSS);

    DEBUG_PRINT("SPI initialized on pins: SCK=");
    DEBUG_PRINT(SX1262_SCK);
    DEBUG_PRINT(", MISO=");
    DEBUG_PRINT(SX1262_MISO);
    DEBUG_PRINT(", MOSI=");
    DEBUG_PRINT(SX1262_MOSI);
    DEBUG_PRINT(", NSS=");
    DEBUG_PRINTLN(SX1262_NSS);

    // Create radio instance
    // Module(cs, irq, rst, gpio, spi, spiSettings)
    radio = new SX1262(new Module(SX1262_NSS, SX1262_DIO1, SX1262_RESET, SX1262_BUSY, *spi));

    DEBUG_PRINTLN("Configuring SX1262 FSK mode...");

    // Heltec V3 has board-managed TCXO (always-on, hardware controlled)
    // Try using beginFSK() with no parameters, then configure manually
    DEBUG_PRINTLN("Initializing FSK mode with defaults...");

    // Call beginFSK() with no parameters to use defaults
    // This avoids TCXO configuration issues
    int state = radio->beginFSK();

    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("FSK mode initialized successfully!");
    } else {
        DEBUG_PRINT("beginFSK() failed, code: ");
        DEBUG_PRINTLN(state);
        return;
    }

    // Now configure WMBus Mode C1 parameters manually
    DEBUG_PRINTLN("Configuring WMBus Mode C1 parameters...");

    // Set frequency for WMBus Mode C1
    state = radio->setFrequency(868.95);
    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("Frequency: 868.95 MHz");
    } else {
        DEBUG_PRINT("setFrequency failed, code: ");
        DEBUG_PRINTLN(state);
    }

    // Set bit rate
    state = radio->setBitRate(100.0);
    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("Bit rate: 100 kbps");
    } else {
        DEBUG_PRINT("setBitRate failed, code: ");
        DEBUG_PRINTLN(state);
    }

    // Set frequency deviation
    state = radio->setFrequencyDeviation(50.0);
    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("Freq deviation: ±50 kHz");
    } else {
        DEBUG_PRINT("setFrequencyDeviation failed, code: ");
        DEBUG_PRINTLN(state);
    }

    // Set RX bandwidth (must use predefined SX1262 values)
    // Valid values: 4.8, 5.8, 7.3, 9.7, 11.7, 14.6, 19.5, 23.4, 29.3, 39.0,
    //               46.9, 58.6, 78.2, 93.8, 117.3, 156.2, 187.2, 234.3, 312.0, 373.6, 467.0 kHz
    // WMBus typically uses 200-300 kHz, so 234.3 is closest
    state = radio->setRxBandwidth(234.3);
    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("RX bandwidth: 234.3 kHz");
    } else {
        DEBUG_PRINT("setRxBandwidth failed, code: ");
        DEBUG_PRINTLN(state);
    }

    // Configure sync word for WMBus preamble (0x543D)
    uint8_t syncWord[] = {0x54, 0x3D};
    state = radio->setSyncWord(syncWord, 2);
    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("Sync word set: 0x543D");
    } else {
        DEBUG_PRINT("Failed to set sync word, code: ");
        DEBUG_PRINTLN(state);
    }

    // Disable CRC checking (WMBus has its own CRC)
    state = radio->setCRC(false);
    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("CRC disabled");
    } else {
        DEBUG_PRINT("Failed to disable CRC, code: ");
        DEBUG_PRINTLN(state);
    }

    // Set preamble length
    state = radio->setPreambleLength(16);
    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("Preamble length set: 16 bits");
    }

    // Enable variable packet length mode (WMBus packets vary in length)
    state = radio->variablePacketLengthMode(255);
    if (state == RADIOLIB_ERR_NONE) {
        DEBUG_PRINTLN("Variable packet length enabled");
    } else {
        DEBUG_PRINT("Failed to set variable packet length, code: ");
        DEBUG_PRINTLN(state);
    }

    // Configure DIO2 as RF switch control (Heltec V3 requirement)
    state = radio->setDio2AsRfSwitch(true);
    if (state != RADIOLIB_ERR_NONE) {
        DEBUG_PRINT("Failed to configure DIO2 RF switch, code: ");
        DEBUG_PRINTLN(state);
    }

// Set receiver gain boost for better sensitivity
// Note: This method may not exist in all RadioLib versions
// If compilation fails, comment out these lines
#ifdef RADIOLIB_SX126X_RX_GAIN_BOOSTED
    state = radio->setRxBoostedGainMode(true);
    if (state != RADIOLIB_ERR_NONE) {
        DEBUG_PRINT("Failed to set RX gain boost, code: ");
        DEBUG_PRINTLN(state);
    }
#endif

    // Disable whitening (WMBus doesn't use whitening)
    state = radio->setWhitening(false);
    if (state != RADIOLIB_ERR_NONE) {
        DEBUG_PRINT("Failed to disable whitening, code: ");
        DEBUG_PRINTLN(state);
    }

    // Set data shaping (Gaussian filter BT=0.5 for cleaner signal)
    state = radio->setDataShaping(RADIOLIB_SHAPING_0_5);
    if (state != RADIOLIB_ERR_NONE) {
        DEBUG_PRINT("Failed to set data shaping, code: ");
        DEBUG_PRINTLN(state);
    }

    DEBUG_PRINTLN("Attaching interrupt handler...");

    // Attach interrupt for packet reception on DIO1
    radio->setDio1Action(onReceive);

    DEBUG_PRINTLN("Starting receiver...");

    // Start receiving
    state = radio->startReceive();
    if (state != RADIOLIB_ERR_NONE) {
        DEBUG_PRINT("Failed to start receiver, code: ");
        DEBUG_PRINTLN(state);
        return;
    }

    DEBUG_PRINTLN("SX1262 ready for WMBus reception");
}

// Check if a frame is available (call frequently from main loop)
bool WaterMeter::isFrameAvailable(void) {
    if (packetReceived) {
        DEBUG_PRINTLN("Packet interrupt detected");

        // Clear the flag
        packetReceived = false;

        // Disable interrupt while processing
        radio->clearDio1Action();

        // Create frame object
        WMBusFrame frame;

        // Receive and process the frame
        receive(&frame);

        // Re-enable interrupt for next packet
        radio->setDio1Action(onReceive);

        // Restart receiver
        int state = radio->startReceive();
        if (state != RADIOLIB_ERR_NONE) {
            DEBUG_PRINT("Failed to restart receiver, code: ");
            DEBUG_PRINTLN(state);
        }

        return frame.isValid;
    }
    return false;
}

// Receive a WMBus frame from the radio
void WaterMeter::receive(WMBusFrame* frame) {
    uint8_t buffer[255];
    int state = radio->readData(buffer, 255);

    if (state == RADIOLIB_ERR_NONE) {
        // Get packet length
        size_t len = radio->getPacketLength();

        DEBUG_PRINT("Received packet, length: ");
        DEBUG_PRINTLN(len);

        // WMBus Mode C1 frame format:
        // Byte 0: Preamble byte 1 (0x54) - already matched by sync word
        // Byte 1: Preamble byte 2 (0x3D) - already matched by sync word
        // Byte 2: Length field (L-field)
        // Byte 3+: Actual frame data

        // Check if we have at least preamble + length field
        if (len >= 3) {
            // Check preamble (sync word should have already matched this)
            if (buffer[0] == 0x54 && buffer[1] == 0x3D) {
                uint8_t payloadLength = buffer[2];

                DEBUG_PRINT("WMBus frame length field: ");
                DEBUG_PRINTLN(payloadLength);

                // Validate length
                if (payloadLength < WMBusFrame::MAX_LENGTH && (payloadLength + 3) <= len) {
                    frame->length = payloadLength;

                    // Copy payload (skip preamble and length field)
                    for (int i = 0; i < payloadLength; i++) {
                        frame->payload[i] = buffer[i + 3];
                    }

                    // Decode and validate the frame
                    frame->decode();

                    if (frame->isValid) {
                        DEBUG_PRINTLN("Valid WMBus frame received!");
                        DEBUG_PRINT("RSSI: ");
                        DEBUG_PRINT(getRSSI());
                        DEBUG_PRINTLN(" dBm");
                    } else {
                        DEBUG_PRINTLN("Frame validation failed (CRC or meter ID mismatch)");
                    }
                } else {
                    DEBUG_PRINTLN("Invalid payload length");
                }
            } else {
                DEBUG_PRINTLN("Invalid preamble (should not happen after sync word match)");
            }
        } else {
            DEBUG_PRINTLN("Packet too short for WMBus frame");
        }
    } else {
        DEBUG_PRINT("Failed to read data, code: ");
        DEBUG_PRINTLN(state);
    }
}

// Get RSSI of last received packet
int16_t WaterMeter::getRSSI() {
    if (radio != nullptr) {
        return radio->getRSSI();
    }
    return 0;
}

// Get SNR of last received packet
float WaterMeter::getSNR() {
    if (radio != nullptr) {
        return radio->getSNR();
    }
    return 0.0;
}
