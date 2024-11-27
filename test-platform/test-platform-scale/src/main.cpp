#include <Arduino.h>
#include <WiFi.h>
#include <AsyncWebSocket.h>
#include <SPIFFS.h>
#include <HX711.h>
#include <EEPROM.h>

#define PIN_BUTTON 23

#ifndef WIFI_SSID
// #define WIFI_SSID "cosmoxd"
// #define WIFI_PASSWORD "cosmolol"
#define WIFI_SSID "Foo"
#define WIFI_PASSWORD "12345678b"
#endif

#define LOADCELL_DOUT_PIN 18
#define LOADCELL_SCK_PIN 23

struct EepromData {
  float scale;
  float offset;
};

int buttonState = 0;
AsyncWebServer server(80);
HX711 loadCell;

void initServerCommands();
void initWifi();
void initHX711();

void block() {
  while (1) {
    NOP();
  }
}

void persistScaleConfiguration() {
  EepromData data;
  data.scale = loadCell.get_scale();
  data.offset = loadCell.get_offset();
  Serial.printf("Persisting loadcell configuration. Scale: %f; offset: %f\n", data.scale, data.offset);
  EEPROM.put(1, data);
  EEPROM.commit();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting");

  initHX711();
  initWifi();
}

void initHX711() {
  loadCell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, 32);

  if(!EEPROM.begin(sizeof(EepromData)*2)) {
    Serial.println("An Error has occurred while mounting EEPROM");
    block();
  }

  EepromData eepromData;
  EEPROM.get(1, eepromData);

  if(eepromData.scale > 0.01) {
    Serial.print("Found epprom data!");
    Serial.printf(" Setting scale to %f\n Setting offset to %f\n", eepromData.scale, eepromData.offset);
    loadCell.set_scale(eepromData.scale);
    loadCell.set_offset(eepromData.offset);
  }
}

void initWifi() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    block();
  }

  Serial.printf("Connecting to %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int status;
  while ((status = WiFi.status()) != WL_CONNECTED) {
    delay(100);
    Serial.printf("%d ", status);
    if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
      Serial.println();
      Serial.println("Failed to connect to WiFi");
      block();
    }
  }
  Serial.println();

  Serial.println("Connected to WiFi");
  WiFi.localIP().printTo(Serial);
  Serial.println();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

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
    float value = loadCell.get_units(count);
    request->send(200, "text/plain", String(value, 10));
  });

  server.on("/tare", HTTP_GET, [](AsyncWebServerRequest *request) {
    loadCell.tare();
    persistScaleConfiguration();
    request->send(200, "text/plain", "1");
  });

  server.on("/set_scale", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebParameter *scaleParam = request->getParam("scale");
    if(scaleParam) {
      float scaleValue = scaleParam->value().toFloat();
      loadCell.set_scale(scaleValue);
      persistScaleConfiguration();
      request->send(200, "text/plain", "1");
    }
    request->send(200, "text/plain", "missing parameter scale");
  });

  server.on("/get_scale", HTTP_GET, [](AsyncWebServerRequest *request) {
    float currentScale = loadCell.get_scale();
    request->send(200, "text/plain", String(currentScale, 20));
  });
}

void loop() {
  delay(1000);
} 