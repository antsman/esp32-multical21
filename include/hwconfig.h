#ifndef __HWCONFIG_H__
#define __HWCONFIG_H__

#if defined(ESP8266)
// Attach CC1101 pins to ESP8266 SPI pins
// VCC   => 3V3
// GND   => GND
// CSN   => D8
// MOSI  => D7
// MISO  => D6
// SCK   => D5
// GD0   => D2  A valid interrupt pin for your platform (defined below this)
// GD2   => not connected
#define CC1101_GDO0 D2  // GDO0 input interrupt pin
#define LED_BUILTIN D4

#elif defined(HELTEC_V3)
// Heltec WiFi LoRa 32 V3 with SX1262 radio
// Radio uses built-in SPI connection on board
// SX1262 Pin Configuration:
// NSS/CS   => GPIO 8
// MOSI     => GPIO 10
// MISO     => GPIO 11
// SCK      => GPIO 9
// RESET    => GPIO 12
// DIO1     => GPIO 14 (interrupt)
// BUSY     => GPIO 13

#define SX1262_NSS   8
#define SX1262_DIO1  14
#define SX1262_RESET 12
#define SX1262_BUSY  13
#define SX1262_MOSI  10
#define SX1262_MISO  11
#define SX1262_SCK   9

// OLED Display (I2C)
#define OLED_SDA     17
#define OLED_SCL     18
#define OLED_RST     21
#define OLED_ADDR    0x3C
#define OLED_VEXT    36  // Power control for display

// Built-in LED (already defined in board variant, included for reference)
// #define LED_BUILTIN  35

#elif defined(ESP32)
// Attach CC1101 pins to ESP32 SPI pins
// VCC   => 3V3
// GND   => GND
// CSN   => 4
// MOSI  => 23
// MISO  => 19
// SCK   => 18
// GD0   => 32  any valid interrupt pin for your platform will do
// GD2   => not connected

// attach CC1101 pins to ESP32 SPI pins
#ifdef ESP32C3_SUPERMINI
#define CC1101_GDO0 10
#define LED_BUILTIN 8
#else
#define CC1101_GDO0 32
#define LED_BUILTIN 2
#endif
#endif

#endif  //__HWCONFIG_H__
