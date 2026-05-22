/*
 * Heltec WiFi LoRa 32 V3 - Basic Hardware Test
 * Tests: Serial output, LED blinking, WiFi connection
 */

#include <Arduino.h>
#include <WiFi.h>

#include "credentials.h"
#include "hwconfig.h"

#define TEST_NAME "Heltec V3 Hardware Test"

void setup() {
    // Initialize serial
    Serial.begin(115200);

    // Wait longer for USB CDC to initialize
    delay(2000);

    // Ensure USB CDC is ready
    while (!Serial && millis() < 5000) {
        delay(100);
    }

    Serial.println("\n\n=================================");
    Serial.println(TEST_NAME);
    Serial.println("=================================\n");

    // Test LED
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println("[TEST] LED Blink Test");
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.print("  LED ON... ");
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("OFF");
        delay(200);
    }
    Serial.println("[PASS] LED test complete\n");

    // Test WiFi
    Serial.println("[TEST] WiFi Connection Test");
    Serial.println("  Scanning for networks...");

    WiFi.mode(WIFI_STA);
    int numNetworks = WiFi.scanNetworks();

    if (numNetworks == 0) {
        Serial.println("[FAIL] No WiFi networks found!");
        return;
    }

    Serial.printf("  Found %d networks:\n", numNetworks);
    for (int i = 0; i < numNetworks && i < 5; i++) {
        Serial.printf("    %d: %s (RSSI: %d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }

    // Try to connect to first available credential
    Serial.println("\n  Attempting to connect to WiFi...");
    bool connected = false;

    for (int i = 0; i < NUM_SSID_CREDENTIALS; i++) {
        Serial.printf("  Trying: %s... ", credentials[i][0]);

        WiFi.begin(credentials[i][0], credentials[i][1]);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            Serial.println(" Connected!");
            Serial.printf("[PASS] WiFi Connected to: %s\n", credentials[i][0]);
            Serial.printf("       IP Address: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("       Signal Strength: %d dBm\n", WiFi.RSSI());
            Serial.printf("       MAC Address: %s\n", WiFi.macAddress().c_str());
            break;
        } else {
            Serial.println(" Failed");
        }
    }

    if (!connected) {
        Serial.println("[FAIL] Could not connect to any WiFi network");
        Serial.println("       Check credentials.h configuration");
    }

    Serial.println("\n=================================");
    Serial.println("Hardware Test Complete");
    Serial.println("=================================\n");
}

void loop() {
    // Blink LED to show running
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);

    // Show WiFi status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[%lu] WiFi: Connected | IP: %s | RSSI: %d dBm\n", millis(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
    } else {
        Serial.printf("[%lu] WiFi: Disconnected\n", millis());
    }
}
