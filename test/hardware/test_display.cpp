/*
 * Display Test for Heltec V3
 *
 * Tests the OLED display functionality by showing:
 * - Initialization screen
 * - WiFi connection simulation
 * - MQTT connection simulation
 * - Water meter data display
 *
 * Build environment: heltec_v3_display_test
 */

#include <Arduino.h>

#ifdef HELTEC_V3
#include "display.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n=== Heltec V3 Display Test ===\n");

    // Initialize display
    Serial.println("1. Initializing display...");
    displayInit();
    delay(3000);

    // Test status display
    Serial.println("2. Testing status display...");
    displayShowStatus("Testing display");
    delay(2000);

    // Test WiFi status
    Serial.println("3. Testing WiFi status...");
    displayShowWifiStatus("MyWiFi", "192.168.1.100");
    delay(3000);

    // Test MQTT connecting
    Serial.println("4. Testing MQTT connecting...");
    displayShowMqttStatus(false, "mqtt.example.com");
    delay(2000);

    // Test MQTT connected
    Serial.println("5. Testing MQTT connected...");
    displayShowMqttStatus(true, "mqtt.example.com");
    delay(2000);

    // Test water meter data
    Serial.println("6. Testing water meter data...");
    displayShowWaterMeterData(123.456, 100.250, 22.5, 18.3);
    delay(5000);

    // Test error display
    Serial.println("7. Testing error display...");
    displayShowError("Test error message");
    delay(2000);

    // Final status
    displayShowStatus("Test complete!");
    Serial.println("\n=== Test Complete ===");
    Serial.println("All display functions tested successfully.");
}

void loop() {
    // Cycle through different displays every 5 seconds
    static unsigned long lastUpdate = 0;
    static int displayState = 0;

    if (millis() - lastUpdate > 5000) {
        lastUpdate = millis();

        switch (displayState) {
            case 0:
                displayShowWaterMeterData(123.456 + random(0, 10) / 10.0, 100.250, 22.5, 18.3);
                break;
            case 1:
                displayShowStatus("Ready");
                break;
            case 2:
                displayShowMqttStatus(true, "mqtt.example.com");
                break;
        }

        displayState = (displayState + 1) % 3;
    }
}

#else
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\nERROR: This test is only for Heltec V3!");
    Serial.println("Please use the heltec_v3_display_test environment.");
}

void loop() {
    delay(1000);
}
#endif
