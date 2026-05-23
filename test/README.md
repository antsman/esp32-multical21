# Test Files

This directory contains hardware validation tests used during the Heltec V3 migration (Phase 1 & 2).

## Hardware Tests

### `hardware/test_heltec.cpp`
**Purpose:** Basic hardware validation for Heltec WiFi LoRa 32 V3
**Tests:**
- Serial communication (115200 baud, hardware UART)
- LED control (GPIO 35)
- WiFi scanning and connection
- Network connectivity (IP assignment, RSSI)

**Usage:**
```ini
# In platformio.ini, use environment:
[env:heltec_v3_test]
build_src_filter = +<*> -<main.cpp> -<WaterMeter.cpp> -<WMBusFrame.cpp>
```

### `hardware/test_sx1262_lora.cpp`
**Purpose:** SX1262 radio validation in LoRa mode
**Tests:**
- SPI communication (pins 9, 10, 11, 8)
- Radio initialization
- DIO1 interrupt functionality
- TCXO and RF switch configuration
- Frequency setting (868.95 MHz)
- Receiver operation

**Usage:**
```ini
# In platformio.ini, use environment:
[env:heltec_v3_radio_test]
build_src_filter = +<*> -<main.cpp> -<WaterMeter.cpp> -<WaterMeter_SX1262.cpp> -<WMBusFrame.cpp> -<test_heltec.cpp>
```

## Notes

- These tests are **not** included in the main production build
- Keep them for future debugging or hardware validation
- Both tests use `credentials.h` for WiFi settings
- Serial output requires hardware UART (USB CDC disabled)

## History

Created during Heltec V3 migration:
- Phase 1: `test_heltec.cpp` - Basic hardware validation
- Phase 2: `test_sx1262_lora.cpp` - Radio hardware validation before FSK implementation
