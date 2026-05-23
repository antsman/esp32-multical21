# OLED Display Support (Heltec V3)

The Heltec WiFi LoRa 32 V3 includes a built-in 0.96" OLED display (128x64 pixels, SSD1306 driver) that can show water meter readings and system status in real-time.

## Features

The display automatically shows:
- **Startup**: Initialization status
- **WiFi Connection**: SSID and IP address
- **MQTT Status**: Connection state and broker address
- **Water Meter Data**: Current reading, monthly start value, room and water temperatures
- **Error Messages**: WiFi or MQTT connection failures

## Hardware Configuration

The display is connected via I2C on the following pins:

| Signal | GPIO | Description |
|--------|------|-------------|
| SDA    | 17   | I2C Data    |
| SCL    | 18   | I2C Clock   |
| RST    | 21   | Display Reset |
| Addr   | 0x3C | I2C Address |

These pins are defined in `include/hwconfig.h`.

## Display Functions

### Core Functions

#### `displayInit()`
Initializes the display with proper reset sequence and default settings.

#### `displayClear()`
Clears the display buffer (call `display.display()` to update screen).

#### `displayShowStatus(const char* status)`
Shows a simple status message:
```
Status:
[Your message]
```

#### `displayShowWifiStatus(const char* ssid, const char* ip)`
Shows WiFi connection information:
```
WiFi Connected
[SSID]
[IP Address]
```

#### `displayShowMqttStatus(bool connected, const char* broker)`
Shows MQTT connection status:
```
MQTT Connected      (or "MQTT Connecting...")
[Broker Address]
Ready               (only when connected)
```

#### `displayShowWaterMeterData(float currentValue, float monthStart, float roomTemp, float waterTemp)`
Shows water meter readings:
```
Water Meter
Current: 123.456 m3
Month: 100.250 m3
Room: 22.5C  Water: 18.3C
```

#### `displayShowError(const char* error)`
Shows an error message:
```
ERROR:
[Error message]
```

## Display Updates

The display updates automatically at these points:

1. **Startup** (`setup()`): Shows "Initializing..." then "Setup complete"
2. **WiFi Connection** (`StateWifiConnect`): Shows "Connecting WiFi..." then WiFi details
3. **MQTT Connection** (`StateMqttConnect`): Shows "MQTT Connecting..." then "MQTT Connected"
4. **Water Meter Reception** (`waterMeterLoop()`): Updates with new meter data when a valid frame is received
5. **Errors**: Shows error messages when WiFi or MQTT connection fails

## Testing

To test the display functionality without needing a water meter:

```bash
# Build and upload display test
~/.platformio/penv/bin/pio run -e heltec_v3_display_test -t upload

# Monitor output
~/.platformio/penv/bin/pio device monitor -e heltec_v3_display_test
```

The test cycles through all display functions:
- Initialization screen
- Status messages
- WiFi connection simulation
- MQTT connection states
- Water meter data with sample values
- Error message display

## Library

The display uses the **ThingPulse SSD1306 driver**:
```
thingpulse/ESP8266 and ESP32 OLED driver for SSD1306 displays @ ^4.4.0
```

This library is automatically installed by PlatformIO when building the `heltec_v3` environment.

## Code Structure

```
include/
  display.h           # Display function declarations
  hwconfig.h          # Pin definitions

src/
  display.cpp         # Display implementation
  main.cpp            # Integration with main application

test/hardware/
  test_display.cpp    # Display test program
```

## Integration in Main Application

Display code is conditionally compiled only for Heltec V3:

```cpp
#ifdef HELTEC_V3
#include "display.h"
#endif

void setup() {
    // ...
#ifdef HELTEC_V3
    displayInit();
#endif
    // ...
}

void waterMeterLoop() {
    if (waterMeter.isFrameAvailable()) {
#ifdef HELTEC_V3
        const WMBusFrame& frame = waterMeter.getLastFrame();
        if (frame.isValid) {
            displayShowWaterMeterData(
                frame.currentValue,
                frame.monthStartValue,
                frame.roomTemp,
                frame.waterTemp
            );
        }
#endif
    }
}
```

## Customization

To customize the display:

1. **Change Font**: Edit `display.cpp` and use different fonts from the SSD1306 library:
   - `ArialMT_Plain_10` (default)
   - `ArialMT_Plain_16`
   - `ArialMT_Plain_24`

2. **Change Layout**: Modify the `displayShow*()` functions in `display.cpp` to adjust positioning and content.

3. **Add New Screens**: Create new display functions following the existing pattern:
   ```cpp
   void displayShowMyScreen() {
       display.clear();
       display.setFont(ArialMT_Plain_10);
       display.drawString(0, 0, "My Screen");
       // ... add more content
       display.display();
   }
   ```

## Troubleshooting

### Display Shows Nothing

1. Check I2C connections (SDA=17, SCL=18)
2. Verify display reset pin (RST=21) is working
3. Check I2C address with a scanner (should be 0x3C)
4. Ensure the ThingPulse library is installed

### Display Shows Garbage

1. Check if display orientation is correct (adjust `display.flipScreenVertically()` in `displayInit()`)
2. Verify SPI pins aren't conflicting with display I2C pins
3. Ensure display is getting proper reset signal

### Display Not Updating

1. Ensure `display.display()` is called after drawing (all functions do this automatically)
2. Check if code is running in a tight loop without delays
3. Verify the display functions are inside `#ifdef HELTEC_V3` blocks

## Future Enhancements

Possible improvements:
- [ ] Add graphs showing water consumption over time
- [ ] Show radio signal strength (RSSI/SNR)
- [ ] Add network status indicators (WiFi bars, MQTT icon)
- [ ] Implement screen saver / power saving mode
- [ ] Add button support for switching between screens
- [ ] Store and display consumption statistics
