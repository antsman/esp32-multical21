# Test Files (Reference Only)

This directory contains hardware validation tests used during the Heltec V3 migration (Phase 1 & 2).

**Status:** ⚠️ These are **reference files only** - no build environments configured.

The migration is complete and these tests were only needed during development. They're kept as:
- Documentation of the validation process
- Reference for future hardware debugging
- Examples if you need to validate another Heltec V3 board

## Hardware Test Files

### `hardware/test_heltec.cpp`
**Purpose:** Basic hardware validation for Heltec WiFi LoRa 32 V3

**Tests:**
- Serial communication (115200 baud, hardware UART)
- LED control (GPIO 35)
- WiFi scanning and connection
- Network connectivity (IP assignment, RSSI)

### `hardware/test_sx1262_lora.cpp`
**Purpose:** SX1262 radio validation in LoRa mode

**Tests:**
- SPI communication (pins 9, 10, 11, 8)
- Radio initialization
- DIO1 interrupt functionality
- TCXO and RF switch configuration
- Frequency setting (868.95 MHz)
- Receiver operation

## How to Run (If Needed)

If you need to run these tests, temporarily add this environment to `platformio.ini`:

```ini
# For hardware test
[env:heltec_v3_test]
platform = espressif32 @ ^6.5.0
board = heltec_wifi_lora_32_V3
framework = arduino
build_flags =
    -DHELTEC_V3=1
    -DARDUINO_USB_MODE=0
    -DARDUINO_USB_CDC_ON_BOOT=0
build_src_filter = +<*> +<../test/hardware/test_heltec.cpp> -<main.cpp> -<WaterMeter.cpp> -<WaterMeter_SX1262.cpp> -<WMBusFrame.cpp>
upload_port = /dev/tty.usbserial-0001
monitor_port = /dev/tty.usbserial-0001
monitor_speed = 115200

# For radio test
[env:heltec_v3_radio_test]
platform = espressif32 @ ^6.5.0
board = heltec_wifi_lora_32_V3
framework = arduino
lib_deps = jgromes/RadioLib @ ^6.6.0
build_flags =
    -DHELTEC_V3=1
    -DARDUINO_USB_MODE=0
    -DARDUINO_USB_CDC_ON_BOOT=0
build_src_filter = +<*> +<../test/hardware/test_sx1262_lora.cpp> -<main.cpp> -<WaterMeter.cpp> -<WaterMeter_SX1262.cpp> -<WMBusFrame.cpp>
upload_port = /dev/tty.usbserial-0001
monitor_port = /dev/tty.usbserial-0001
monitor_speed = 115200
```

Then run: `pio run -e heltec_v3_test -t upload` or `pio run -e heltec_v3_radio_test -t upload`

## Notes

- Both tests require `src/credentials.h` with WiFi settings
- Serial output uses hardware UART (USB CDC disabled)
- Remove the test environment from platformio.ini after use

## History

Created during Heltec V3 migration (May 2026):
- **Phase 1:** `test_heltec.cpp` - Basic hardware validation
- **Phase 2:** `test_sx1262_lora.cpp` - Radio validation before FSK implementation
- Migration completed successfully, tests archived as reference
