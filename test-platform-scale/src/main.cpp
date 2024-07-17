#include <Arduino.h>
#include <WiFi.h>
#include <AsyncWebSocket.h>
#include <SPIFFS.h>
#include <HX711.h>

#define PIN_BUTTON 23

#define WIFI_SSID "UPCA6C2546"
#define WIFI_PASSWORD "especb2skFep"

#define LOADCELL_DOUT_PIN 18
#define LOADCELL_SCK_PIN 23

int buttonState = 0;
AsyncWebServer server(80);
HX711 scale;

void initServerCommands();
void initWifi();
void initHX711();

void block() {
  while (1) {
    NOP();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting");

  initHX711();
  initWifi();
}

void initHX711() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, 32);
}

void initWifi() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
    }

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int status;
    while ((status = WiFi.status()) != WL_CONNECTED) {
        delay(100);
        Serial.printf("%d\n", status);
        if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
            Serial.println("Failed to connect to WiFi");
            block();
        }
    }

    Serial.println("Connected to WiFi");
    WiFi.localIP().printTo(Serial);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.serveStatic("/", SPIFFS, "/");

    initServerCommands();

    server.begin();
}

void initServerCommands() {
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncWebParameter *countParam = request->getParam("count");
      int count = 1;
      if(countParam) {
        count = countParam->value().toInt();
      }
      float value = scale.get_units(count);
      request->send(200, "text/plain", String(value, 10));
    });

    server.on("/tare", HTTP_GET, [](AsyncWebServerRequest *request) {
      scale.tare();
      request->send(200, "text/plain", "1");
    });

    server.on("/set_scale", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncWebParameter *scaleParam = request->getParam("scale");
      if(scaleParam) {
        scale.set_scale(scaleParam->value().toFloat());
        request->send(200, "text/plain", "1");
      }
      request->send(200, "text/plain", "missing parameter scale");
    });
}

void loop() {
  delay(100);
  NOP();
} 