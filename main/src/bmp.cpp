#include "bmp.h"

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

// 1019.91 is avg Pressure in Cracow
float constexpr SEALEVELPRESSURE_HPA = 1019.91;
std::uint8_t constexpr CHIP_BME = 0x60;
[[maybe_unused]] std::uint8_t constexpr CHIP_BMP = 0x58;
std::uint8_t constexpr CHIP_ADDR = 0x76;

namespace bmp {

Adafruit_BMP280 bmp_obj;
bool init_success = false;
std::optional<Data> data;

void init() {
    init_success = bmp_obj.begin(CHIP_ADDR, CHIP_BME);
    if (!init_success) {
        Serial.println("Viable sensor BMP280 not found, check wiring!");
    }
}

void get_bmp([[maybe_unused]] void *pvParameters) {
    for (;;) {
        if (!init_success) {
            data = std::nullopt;
        } else {
            data = Data{.temperature = bmp_obj.readTemperature(),
                        .pressure = bmp_obj.readPressure(),
                        .altitude = bmp_obj.readAltitude()};
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
void print_data([[maybe_unused]] void *pvParameters) {
    for (;;) {
        pretty_print(data);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void pretty_print(std::optional<Data> const &bmp_data) {
    if (!bmp_data) {
        Serial.println("BMP data unavailable.");
    } else {
        Serial.print("[Temperature] ");
        Serial.print(data->temperature);
        Serial.println("°C");

        Serial.print("[Pressure] ");
        Serial.print(data->get_as_hpa());
        Serial.println("hPa");

        Serial.print("[Altitude] ");
        Serial.print(data->altitude);
        Serial.println("m");

        Serial.println();
    }
}

float Data::get_as_hpa() { return bmp_obj.readPressure() / 100.0F; }
} // namespace bmp
