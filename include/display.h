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
void displayShowWaterMeterData(float currentValue, float monthStart, float roomTemp, float waterTemp);
void displayShowError(const char* error);

#endif  // HELTEC_V3

#endif  // __DISPLAY_H__
