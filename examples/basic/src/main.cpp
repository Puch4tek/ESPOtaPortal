#include <Arduino.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <OtaPortal.h>

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

WebServer server(80);
OtaPortal portal;

void setup() {
    Serial.begin(115200);
    LittleFS.begin(true);
    delay(2000);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("Connecting to %s", WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
    }
    Serial.printf("\nConnected. Open http://%s/ota\n", WiFi.localIP().toString().c_str());

    portal.setDeviceName("ESP32-C3");
    portal.setDeviceId("example-device");
    portal.setCredentials("admin", "change-this-password");
    portal.setOnStart([]() { Serial.println("OTA started"); });
    portal.setOnEnd([]() { Serial.println("OTA finished"); });
    portal.begin(server);
    server.begin();
}

void loop() {
    server.handleClient();
}
