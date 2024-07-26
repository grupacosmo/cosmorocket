#include "loadcell.h"
#include <AsyncWebSocket.h>
#include <HX711.h>
#include <EEPROM.h>

namespace {
    struct LoadCellConfiguration {
        float scale;
        float offset;
    };

    HX711 loadCell;

    constexpr int loadCellDataAddress = 0;

    constexpr int LOADCELL_DOUT_PIN = 18;
    constexpr int LOADCELL_SCK_PIN = 23;



    void persistScaleConfiguration() {
        LoadCellConfiguration data;
        data.scale = loadCell.get_scale();
        data.offset = loadCell.get_offset();
        Serial.printf("Persisting loadcell configuration. Scale: %f; offset: %f\n", data.scale, data.offset);
        EEPROM.put(loadCellDataAddress, data);
        EEPROM.commit();
    }
}

namespace LoadCell {
    // true - success; false - fail
    bool initHX711() 
    {
        loadCell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, 32);

        if(!EEPROM.begin(sizeof(LoadCellConfiguration))) {
            Serial.println("An Error has occurred while mounting EEPROM");
            return false;
        }

        LoadCellConfiguration eepromData;
        EEPROM.get(loadCellDataAddress, eepromData);

        if(eepromData.scale > 0.01) {
            Serial.print("Found epprom data!");
            Serial.printf(" Setting scale to %f\n Setting offset to %f\n", eepromData.scale, eepromData.offset);
            loadCell.set_scale(eepromData.scale);
            loadCell.set_offset(eepromData.offset);
        }

        return true;
    }

    void initLoadCellEndpoints(AsyncWebServer &server) 
    {
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
}