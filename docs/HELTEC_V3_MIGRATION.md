# Heltec WiFi LoRa 32 V3 Migration Guide

## Overview

This guide documents the migration from **ESP32-C3 Super Mini + CC1101** to **Heltec WiFi LoRa 32 V3** for reading Kamstrup Multical 21 water meter data via WMBus Mode C1.

## Hardware Comparison

### Current Setup (ESP32-C3 + CC1101)
- **MCU**: ESP32-C3 (RISC-V single-core, 160 MHz)
- **Radio**: CC1101 (external SPI module)
- **Frequency**: 868 MHz
- **Interface**: SPI (pins 4-7, 10)
- **Components**: 2 separate modules + wiring

### Target Setup (Heltec V3)
- **MCU**: ESP32-S3FN8 (Xtensa dual-core, 240 MHz)
- **Radio**: SX1262 (integrated LoRa/FSK transceiver)
- **Frequency**: 433/868/915 MHz bands
- **Display**: 0.96" OLED (128x64)
- **Power**: USB-C or battery (with management)
- **Components**: Single integrated board

### Advantages
✅ Single integrated board (no external wiring)
✅ More powerful MCU (dual-core ESP32-S3)
✅ Built-in OLED display for local monitoring
✅ Higher sensitivity radio (-134 dBm @ SF12)
✅ Optional battery operation with built-in management
✅ USB-C connector for modern power/programming

## Migration Scope

### Core Changes
1. **Radio Driver**: Replace CC1101 SPI code with SX1262 FSK configuration via RadioLib
2. **Pin Mapping**: Update GPIO definitions for Heltec V3 hardware
3. **Build System**: Add new PlatformIO environment for ESP32-S3
4. **Libraries**: Add RadioLib dependency for SX1262 support

### Optional Enhancements
5. **Display**: Add OLED support for local status/readings (U8g2 library)
6. **Battery**: Power management and voltage monitoring (if battery powered)

### Excluded (USB-Powered)
❌ Deep sleep optimization
❌ Battery voltage monitoring
❌ Low-power modes

## Technical Feasibility

### SX1262 FSK Capabilities

The SX1262 **fully supports** WMBus Mode C1 requirements:

| WMBus C1 Requirement | SX1262 Capability | Status |
|---------------------|-------------------|---------|
| **Frequency**: 868.95 MHz | 150-960 MHz range | ✅ Supported |
| **Bit Rate**: 100 kbps | 0.6-300 kbps range | ✅ Supported |
| **Freq Deviation**: ±50 kHz | 0-200 kHz range | ✅ Supported |
| **Modulation**: 2-FSK | FSK mode via `beginFSK()` | ✅ Supported |
| **Sync Word**: 0x543D | 1-8 byte sync words | ✅ Supported |
| **Variable Length**: Yes | `variablePacketLengthMode()` | ✅ Supported |

### RadioLib Support

**Library**: [RadioLib](https://github.com/jgromes/RadioLib) by Jan Gromeš
**Documentation**: https://jgromes.github.io/RadioLib/class_s_x1262.html

RadioLib provides complete FSK support for SX1262 with:
- `beginFSK()` - Initialize FSK mode with frequency, bit rate, deviation
- `setSyncWord()` - Configure preamble/sync word detection
- `setBitRate()` - Set data rate (0.6-300 kbps)
- `setFrequencyDeviation()` - Set FSK deviation (0-200 kHz)
- `variablePacketLengthMode()` - Enable variable packet lengths
- `setRxBandwidth()` - Configure receiver bandwidth

### Real-World Evidence

Multiple active projects successfully use SX1262 for WMBus reception:
- [esphome-wmbus-bridge-rawonly](https://github.com/Kustonium/esphome-wmbus-bridge-rawonly) - Active WMBus bridge
- [esphome-components-wmbus](https://github.com/HenrikBurton/esphome-components-wmbus) - ESPHome integration

## Heltec V3 Hardware Details

### Pin Mapping

**SX1262 Radio Pins** (typically on Heltec V3):
```
NSS/CS:   GPIO 8
RESET:    GPIO 12
BUSY:     GPIO 13
DIO1:     GPIO 14 (interrupt)
SCK:      GPIO 9  (hardware SPI)
MOSI:     GPIO 10 (hardware SPI)
MISO:     GPIO 11 (hardware SPI)
```

**OLED Display** (I2C):
```
SDA:      GPIO 17
SCL:      GPIO 18
Address:  0x3C
```

**Built-in LED**:
```
LED:      GPIO 35
```

**Note**: Verify exact pins from [Heltec official documentation](https://resource.heltec.cn/download/WiFi_LoRa_32_V3/HTIT-WB32LA(F)_V3.pdf)

### Critical Configuration

The Heltec V3 **requires** specific settings for SX1262:
- **TCXO**: Enabled (board has 32 MHz TCXO)
- **DIO2 RF Switch**: Enabled (DIO2 controls RF switching)
- **RX Gain**: Boosted (for better sensitivity)

## Implementation Plan

### Phase 1: Environment Setup ✅ COMPLETE
- [x] Research SX1262 FSK capabilities
- [x] Add Heltec V3 environment to `platformio.ini`
- [x] Configure ESP32-S3 build flags
- [x] Add RadioLib dependency
- [x] Test basic WiFi/MQTT connectivity

**Status**: Complete (Commit: 5933176)
**Hardware Validated**:
- Board: Heltec WiFi LoRa 32 V3
- Chip: ESP32-S3 (MAC: 9C:13:9E:E7:1D:14)
- Serial: Working (CP2102 at 115200 baud)
- LED: Working (GPIO 35)
- WiFi: Working (connected at 192.168.1.203, RSSI: -88 to -93 dBm)
- Test Environment: `heltec_v3_test` created and validated

### Phase 2: Radio Driver Migration ⚠️ BLOCKED (80% complete)
- [x] Update `hwconfig.h` with Heltec V3 pin definitions
- [x] Create `WaterMeter_SX1262.cpp` using RadioLib API
- [x] Implement sync word detection (0x543D preamble)
- [x] Port interrupt handling from GDO0 to DIO1
- [x] Validate SX1262 hardware (LoRa mode works)
- [ ] **BLOCKED**: Configure FSK mode for WMBus Mode C1

**Status**: Blocked at FSK initialization (Commit: fe4ceeb)

**What Works**:
- ✅ SX1262 hardware initialization (LoRa mode via `begin()`)
- ✅ SPI communication (pins: SCK=9, MISO=11, MOSI=10, NSS=8)
- ✅ Frequency setting (868.95 MHz)
- ✅ DIO2 RF switch configuration
- ✅ DIO1 interrupt system (tested, functional)
- ✅ Receiver operation (stable 120+ seconds)
- ✅ Driver architecture complete

**Blocker**:
- ❌ `beginFSK()` fails with error -104 (RADIOLIB_ERR_INVALID_TCXO_VOLTAGE)
- Tested TCXO voltages: 3.3V, 1.8V, 2.2V, 2.4V, 3.0V - all fail
- `begin()` works without TCXO parameter (suggests board manages TCXO internally)
- FSK vs LoRa mode switching not yet solved

**Investigation Needed**:
1. RadioLib Heltec V3 board variant FSK examples
2. Manual FSK configuration after `begin()` (individual setters)
3. RadioLib version compatibility with Heltec V3
4. Direct SX126x register access for FSK mode
5. Community solutions for Heltec V3 + RadioLib FSK

**Test Environment**: `heltec_v3_radio_test` validates hardware

### Phase 3: Testing & Validation (3-4 hours)
- [ ] Test WMBus frame reception from Multical 21
- [ ] Verify AES-128 decryption still works
- [ ] Confirm MQTT publishing to Home Assistant
- [ ] Validate CRC checking
- [ ] Compare signal strength (RSSI) with CC1101

### Phase 4: Optional Display (2-3 hours)
- [ ] Add U8g2 library for OLED support
- [ ] Display connection status (WiFi/MQTT)
- [ ] Show last meter reading and timestamp
- [ ] Display RSSI and frame statistics
- [ ] Add error state visualization

**Total Estimated Time: 12-17 hours**

## SX1262 Configuration Example

### Basic FSK Initialization

```cpp
#include <RadioLib.h>

// Define pins for Heltec V3
#define NSS_PIN     8
#define DIO1_PIN    14
#define RESET_PIN   12
#define BUSY_PIN    13

// Create radio instance
SX1262 radio = new Module(NSS_PIN, DIO1_PIN, RESET_PIN, BUSY_PIN);

void setupRadio() {
    // Initialize FSK mode for WMBus Mode C1
    int state = radio.beginFSK(
        868.95,      // Frequency (MHz) - WMBus Mode C1
        100.0,       // Bit rate (kbps) - WMBus standard
        50.0,        // Frequency deviation (kHz) - ±50 kHz
        200.0,       // RX bandwidth (kHz)
        14,          // TX power (dBm) - not used for RX only
        16,          // Preamble length (bits)
        1.8,         // TCXO voltage (V) - Heltec V3 uses 1.8V
        false        // Use DC-DC regulator (not LDO)
    );

    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to initialize radio: ");
        Serial.println(state);
        return;
    }

    // Configure sync word for WMBus preamble (0x543D)
    uint8_t syncWord[] = {0x54, 0x3D};
    radio.setSyncWord(syncWord, 2);

    // Enable variable packet length mode
    radio.variablePacketLengthMode(255);

    // Configure DIO2 as RF switch control (Heltec V3 requirement)
    radio.setDio2AsRfSwitch(true);

    // Set receiver gain boost for better sensitivity
    radio.setRxBoostedGainMode(true);

    // Attach interrupt for packet reception
    radio.setDio1Action(packetReceived);

    // Start receiving
    radio.startReceive();
}

// Interrupt handler
volatile bool packetAvailable = false;

void packetReceived(void) {
    packetAvailable = true;
}

// Main loop check
void loop() {
    if (packetAvailable) {
        packetAvailable = false;

        uint8_t buffer[255];
        int state = radio.readData(buffer, 255);

        if (state == RADIOLIB_ERR_NONE) {
            // Process WMBus frame
            processWMBusFrame(buffer, radio.getPacketLength());
        }
    }
}
```

## Files to Modify

| File | Change Type | Description |
|------|------------|-------------|
| `platformio.ini` | Major | Add Heltec V3 environment, RadioLib library |
| `include/hwconfig.h` | Major | Add Heltec V3 pin definitions (SX1262, OLED) |
| `src/WaterMeter.cpp` | Complete rewrite | Replace CC1101 code with SX1262/RadioLib |
| `include/WaterMeter.h` | Major | Update class interface for RadioLib API |
| `src/main.cpp` | Minor | Optional: add OLED display support |
| `README.md` | Update | New hardware, wiring, build instructions |

## Potential Challenges & Solutions

### Challenge 1: Preamble/Sync Word Detection
**Issue**: WMBus uses specific 0x543D preamble pattern
**Solution**: Use RadioLib's `setSyncWord()` with 2-byte pattern, verify timing matches WMBus spec

### Challenge 2: Manchester Encoding
**Issue**: WMBus Mode C1 may use Manchester encoding
**Solution**: Check if SX1262 handles in hardware; if not, implement software decoding after reception

### Challenge 3: TCXO Configuration
**Issue**: Incorrect TCXO settings cause "silent RX" (radio initializes but receives nothing)
**Solution**: Set `tcxoVoltage=1.8` in `beginFSK()` and verify with test transmission

### Challenge 4: Packet Timing Differences
**Issue**: SX1262 interrupt behavior differs from CC1101's GDO0
**Solution**: Use logic analyzer to verify timing, adjust interrupt edge detection if needed

### Challenge 5: Reception Sensitivity
**Issue**: Range/sensitivity may differ from CC1101
**Solution**:
- Enable boosted gain mode (`setRxBoostedGainMode(true)`)
- Compare RSSI values between implementations
- Adjust RX bandwidth if needed

## Testing Strategy

### 1. Hardware Verification
- [ ] Flash basic blink sketch to confirm board works
- [ ] Test WiFi connectivity with simple example
- [ ] Verify OLED display with U8g2 test sketch

### 2. Radio Functionality
- [ ] Initialize SX1262 in FSK mode
- [ ] Verify radio enters RX state (check status registers)
- [ ] Monitor interrupt pin activity during meter transmissions
- [ ] Capture raw data and compare with CC1101 reference

### 3. WMBus Reception
- [ ] Detect preamble (0x543D) in received data
- [ ] Verify packet length field matches
- [ ] Validate CRC of received frames
- [ ] Compare frame contents with CC1101 implementation

### 4. End-to-End Validation
- [ ] Confirm AES-128 decryption produces correct values
- [ ] Verify MQTT messages match expected format
- [ ] Test Home Assistant integration still works
- [ ] Run for 24h to ensure stability

## Migration Checklist

### Pre-Migration
- [x] Document current CC1101 configuration
- [x] Research SX1262 capabilities
- [x] Verify RadioLib FSK support
- [x] Identify Heltec V3 pinout
- [ ] Order/obtain Heltec V3 hardware

### Development
- [ ] Create `heltec-v3-migration` branch
- [ ] Add new PlatformIO environment
- [ ] Implement SX1262 driver using RadioLib
- [ ] Port WMBus frame processing logic
- [ ] Add OLED display support (optional)
- [ ] Update documentation and README

### Testing
- [ ] Verify radio initialization succeeds
- [ ] Capture WMBus frames from meter
- [ ] Validate decryption works correctly
- [ ] Test MQTT publishing
- [ ] Compare RSSI/range with CC1101
- [ ] 24-hour stability test

### Deployment
- [ ] Update README with new hardware
- [ ] Create wiring diagram for Heltec V3
- [ ] Document any configuration changes
- [ ] Merge to master branch
- [ ] Tag release (e.g., v2.0.0-heltec-v3)

## References

### Hardware
- [Heltec WiFi LoRa 32 V3 Official Page](https://heltec.org/project/wifi-lora-32-v3/)
- [Heltec V3 Pinout Diagram](https://resource.heltec.cn/download/WiFi_LoRa_32_V3/HTIT-WB32LA(F)_V3.pdf)

### Software Libraries
- [RadioLib GitHub](https://github.com/jgromes/RadioLib)
- [RadioLib SX1262 Documentation](https://jgromes.github.io/RadioLib/class_s_x1262.html)
- [U8g2 OLED Library](https://github.com/olikraus/u8g2)

### WMBus Resources
- [WMBus Protocol Specification](https://www.m-bus.com/)
- [ESPHome WMBus Bridge](https://github.com/Kustonium/esphome-wmbus-bridge-rawonly)
- [WMBus MQTT Bridge](https://github.com/Kustonium/homeassistant-wmbus-mqtt-bridge)

### Original Project
- [chester4444/esp-multical21](https://github.com/chester4444/esp-multical21) - Original CC1101 implementation
- [pthalin/esp32-multical21](https://github.com/pthalin/esp32-multical21) - Fork this project is based on

## Support

For issues specific to this migration:
- Check existing issues in the repository
- Verify Heltec V3 hardware compatibility
- Test with known-working RadioLib examples first
- Compare RSSI/sensitivity with CC1101 baseline

## Hardware Test Results

### Test Environment: `heltec_v3_test`

**Date**: 2026-05-22
**Board**: Heltec WiFi LoRa 32 V3
**Firmware**: `src/test_heltec.cpp`

#### Test Results Summary

| Component | Status | Details |
|-----------|--------|---------|
| **Serial Communication** | ✅ PASS | CP2102 USB-UART bridge at 115200 baud |
| **LED Control** | ✅ PASS | GPIO 35, blink test successful |
| **WiFi Scanning** | ✅ PASS | Found 10 networks, RSSI detection working |
| **WiFi Connection** | ✅ PASS | Connected to SSID: sipelgamees |
| **Network Info** | ✅ PASS | IP: 192.168.1.203, MAC: 9C:13:9E:E7:1D:14 |
| **Signal Strength** | ✅ PASS | RSSI: -88 to -93 dBm (good signal) |
| **Continuous Operation** | ✅ PASS | Status updates every 2 seconds |

#### Serial Output Sample

```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021

=================================
Heltec V3 Hardware Test
=================================

[TEST] LED Blink Test
  LED ON... OFF
  LED ON... OFF
  LED ON... OFF
  LED ON... OFF
  LED ON... OFF
[PASS] LED test complete

[TEST] WiFi Connection Test
  Scanning for networks...
  Found 10 networks:
    1: sipelgamees (RSSI: -44 dBm)
    2: herilane (RSSI: -46 dBm)
    ...

  Attempting to connect to WiFi...
  Trying: sipelgamees... ...... Connected!
[PASS] WiFi Connected to: sipelgamees
       IP Address: 192.168.1.203
       Signal Strength: -92 dBm
       MAC Address: 9C:13:9E:E7:1D:14

=================================
Hardware Test Complete
=================================

[15662] WiFi: Connected | IP: 192.168.1.203 | RSSI: -90 dBm
[17663] WiFi: Connected | IP: 192.168.1.203 | RSSI: -93 dBm
...
```

#### Notes

- **Serial Setup**: Hardware UART used instead of USB CDC for stable output
- **USB CDC Issue**: ESP32-S3 switches from ROM bootloader UART to USB CDC after boot, causing terminal disconnection
- **Solution**: Disabled USB CDC (`ARDUINO_USB_CDC_ON_BOOT=0`) for development/testing
- **Production**: Can re-enable USB CDC once stable

## Troubleshooting

### SX1262 FSK Initialization Fails (Error -104)

**Symptom**: `beginFSK()` returns error code -104 (RADIOLIB_ERR_INVALID_TCXO_VOLTAGE)

**Tested Solutions** (all failed):
- ❌ TCXO voltages: 1.8V, 2.2V, 2.4V, 3.0V, 3.3V
- ❌ `beginFSK()` with explicit parameters
- ❌ `beginFSK()` with TCXO=0 (board-managed)
- ❌ `beginFSK()` without TCXO parameter

**What Works**:
- ✅ `begin()` for LoRa mode (no TCXO parameter)
- ✅ Hardware fully functional in LoRa mode
- ✅ All individual setters work (frequency, RF switch, etc.)

**Hypothesis**:
Heltec V3 board variant may have specific TCXO handling that conflicts with RadioLib's FSK initialization. The TCXO might be board-controlled and not software-configurable.

**Potential Solutions to Investigate**:
1. **RadioLib board variants**: Check if there's a Heltec-specific initialization sequence
2. **Manual mode switch**: Call `begin()`, then manually configure FSK registers
3. **RadioLib versions**: Try older/newer versions that might handle Heltec V3 differently
4. **Direct register access**: Use SX126x commands to bypass RadioLib's validation
5. **Community**: Search for RadioLib + Heltec V3 + FSK examples

**Workaround Status**: None yet - blocking WMBus reception

## License

This migration maintains the original GPL-3.0 license from the upstream project.

---

**Status**: Phase 1 Complete ✅ | Phase 2 Blocked ⚠️ (80% done)
**Last Updated**: 2026-05-22
**Branch**: `heltec-v3-migration`
**Commits**: 8 total
- Phase 1: Hardware validation complete
- Phase 2: SX1262 driver 80% complete, blocked at FSK initialization
**Next Step**: Resolve RadioLib FSK + Heltec V3 TCXO compatibility
