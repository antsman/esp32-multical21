#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#ifdef HELTEC_V3

#include <SSD1306Wire.h>

#include "hwconfig.h"

extern SSD1306Wire display;

void displayInit();
void displayClear();
void displayShowStatus(const char* status);
void displayShowWifiStatus(const char* ssid, const char* ip);
void displayShowMqttStatus(bool connected, const char* broker);
void displayShowWaterMeterData(float currentValue, float monthStart, float roomTemp, float waterTemp, int16_t rssi);
void displayShowConnectionInfo(const char* ssid, int8_t wifiRssi, const char* ip, const char* mqttHost, uint16_t mqttPort);
void displayShowError(const char* error);

// Button handling
void displayButtonInit();
bool displayButtonPressed();
uint8_t displayGetCurrentScreen();
void displayNextScreen();

#endif  // HELTEC_V3

#endif  // __DISPLAY_H__
