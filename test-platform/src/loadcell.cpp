#include "loadcell.h"
#include "ESPAsyncWebServer.h"
#include <AsyncWebSocket.h>
#include <EEPROM.h>
#include <HX711.h>

namespace {
struct LoadCellConfiguration {
    float scale;
    float offset;
};

HX711 load_cell;

constexpr int load_cell_data_address = 0;

constexpr int loadcell_dout_pin = 18;
constexpr int loadcell_sck_pin = 23;

auto persist_scale_configuration() -> void {
    LoadCellConfiguration data;
    data.scale = load_cell.get_scale();
    data.offset = load_cell.get_offset();
    Serial.printf("Saving loadcell configuration. Scale: %f; offset: %f\n",
                  data.scale, data.offset);
    EEPROM.put(load_cell_data_address, data);
    EEPROM.commit();
}
} // namespace

namespace LoadCell {
/// returns true on init succes, false on failure
auto init_hx711() -> bool {
    load_cell.begin(loadcell_dout_pin, loadcell_sck_pin, 32);

    if (!EEPROM.begin(sizeof(LoadCellConfiguration))) {
        Serial.println("An error has occurred while mounting EEPROM");
        return false;
    }

    LoadCellConfiguration eeprom_data;
    EEPROM.get(load_cell_data_address, eeprom_data);

    if (eeprom_data.scale > 0.01) {
        Serial.println("Finding EEPROM data succeeded.");
        Serial.printf("Setting scale to %f.\nSetting offset to %f\n",
                      eeprom_data.scale, eeprom_data.offset);
        load_cell.set_scale(eeprom_data.scale);
        load_cell.set_offset(eeprom_data.offset);
    }

    return true;
}

auto init_load_cell_endpoints(AsyncWebServer &server) -> void {
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebParameter *count_param = request->getParam("count");
        int count = 1;
        if (count_param) {
            count = count_param->value().toInt();
        }
        float value = load_cell.get_units(count);
        request->send(200, "text/plain", String(value, 10));
    });

    server.on("/tare", HTTP_GET, [](AsyncWebServerRequest *request) {
        load_cell.tare();
        persist_scale_configuration();
        request->send(200, "text/plain", "1");
    });

    server.on("/set_scale", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebParameter *scale_param = request->getParam("scale");
        if (scale_param) {
            float scale_value = scale_param->value().toFloat();
            load_cell.set_scale(scale_value);
            persist_scale_configuration();
            request->send(200, "text/plain", "1");
        }
        request->send(200, "text/plain", "missing parameter scale");
    });

    server.on("/get_scale", HTTP_GET, [](AsyncWebServerRequest *request) {
        float current_scale = load_cell.get_scale();
        request->send(200, "text/plain", String(current_scale, 20));
    });
}
} // namespace LoadCell
