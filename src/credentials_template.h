#ifndef CREDENTIALS_H
#define CREDENTIALS_H
#include <Arduino.h>

// Wifi settings: SSID, PW, MQTT broker, MQTT port
#define NUM_SSID_CREDENTIALS 1
static const char* credentials[NUM_SSID_CREDENTIALS][4] =
    // SSID,        PW,           MQTT,             PORT (use "0" or "" for default 1883)
    {{"wifi name", "wifi pass", "192.168.1.xxx", "1883"}};

const char mqtt_user[] = "mosquitto-user";
const char mqtt_pass[] = "mosquitto-pass!";

const uint8_t meterId[4] = {0x00, 0x00, 0x00, 0x00};  // Multical21 serial. Printed as hex on meter.
const uint8_t key[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // AES-128 key. Ask your service provider.

#endif
