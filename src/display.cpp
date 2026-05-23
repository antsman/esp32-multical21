#ifdef HELTEC_V3

#include "display.h"

SSD1306Wire display(OLED_ADDR, OLED_SDA, OLED_SCL);

// Button state
static volatile bool buttonPressed = false;
static unsigned long lastButtonPress = 0;
static const unsigned long DEBOUNCE_MS = 200;
static uint8_t currentScreen = 0;  // 0 = meter data, 1 = connection info

// Button interrupt handler
void IRAM_ATTR buttonISR() {
    unsigned long now = millis();
    if (now - lastButtonPress > DEBOUNCE_MS) {
        buttonPressed = true;
        lastButtonPress = now;
    }
}

void displayInit() {
    // Enable power to display (Vext - active LOW)
    pinMode(OLED_VEXT, OUTPUT);
    digitalWrite(OLED_VEXT, LOW);
    delay(100);  // Wait for power to stabilize

    // Reset display
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(50);
    digitalWrite(OLED_RST, HIGH);
    delay(50);

    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    display.clear();
    display.drawString(0, 0, "Water Meter");
    display.drawString(0, 12, "Initializing...");
    display.display();
}

void displayClear() {
    display.clear();
}

void displayShowStatus(const char* status) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Status:");
    display.drawString(0, 12, status);
    display.display();
}

void displayShowWifiStatus(const char* ssid, const char* ip) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "WiFi Connected");
    display.drawString(0, 12, ssid);
    display.drawString(0, 24, ip);
    display.display();
}

void displayShowMqttStatus(bool connected, const char* broker) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    if (connected) {
        display.drawString(0, 0, "MQTT Connected");
        display.drawString(0, 12, broker);
        display.drawString(0, 24, "Ready");
    } else {
        display.drawString(0, 0, "MQTT Connecting...");
        display.drawString(0, 12, broker);
    }
    display.display();
}

void displayShowWaterMeterData(float currentValue, float monthStart, float roomTemp, float waterTemp, int16_t rssi) {
    display.clear();
    display.setFont(ArialMT_Plain_10);

    char buf[32];

    display.drawString(0, 0, "Water Meter");

    snprintf(buf, sizeof(buf), "Current: %.3f m3", currentValue);
    display.drawString(0, 11, buf);

    snprintf(buf, sizeof(buf), "Month: %.3f m3", monthStart);
    display.drawString(0, 22, buf);

    snprintf(buf, sizeof(buf), "Room: %.1fC  Water: %.1fC", roomTemp, waterTemp);
    display.drawString(0, 33, buf);

    // WMBus signal strength
    snprintf(buf, sizeof(buf), "RSSI: %d dBm", rssi);
    display.drawString(0, 44, buf);

    display.display();
}

void displayShowConnectionInfo(const char* ssid, int8_t wifiRssi, const char* ip, const char* mqttHost, uint16_t mqttPort) {
    display.clear();
    display.setFont(ArialMT_Plain_10);

    char buf[32];

    display.drawString(0, 0, "Connection Info");

    snprintf(buf, sizeof(buf), "WiFi: %s", ssid);
    display.drawString(0, 11, buf);

    snprintf(buf, sizeof(buf), "RSSI: %d dBm", wifiRssi);
    display.drawString(0, 22, buf);

    snprintf(buf, sizeof(buf), "IP: %s", ip);
    display.drawString(0, 33, buf);

    snprintf(buf, sizeof(buf), "MQTT:  %s:%d", mqttHost, mqttPort);
    display.drawString(0, 44, buf);

    display.display();
}

void displayShowError(const char* error) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "ERROR:");
    display.drawString(0, 12, error);
    display.display();
}

// Button functions
void displayButtonInit() {
    pinMode(USER_BUTTON, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(USER_BUTTON), buttonISR, FALLING);
}

bool displayButtonPressed() {
    if (buttonPressed) {
        buttonPressed = false;
        return true;
    }
    return false;
}

uint8_t displayGetCurrentScreen() {
    return currentScreen;
}

void displayNextScreen() {
    currentScreen = (currentScreen + 1) % 2;  // Cycle between 0 and 1
}

#endif  // HELTEC_V3
