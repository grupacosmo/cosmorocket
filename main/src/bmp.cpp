#include "bmp.h"

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

// 1019.91 is avg Pressure in Cracow
[[maybe_unused]] constexpr float SEALEVELPRESSURE_HPA = 1019.91;
constexpr std::uint8_t CHIP_BME = 0x60;
[[maybe_unused]] constexpr std::uint8_t CHIP_BMP = 0x58;
constexpr std::uint8_t CHIP_ADDR = 0x76;

namespace {
Adafruit_BMP280 bmp_obj;
bool init_success = false;

bmp::Data data;
} // namespace

namespace bmp {

void init() {
    init_success = bmp_obj.begin(CHIP_ADDR, CHIP_BME);
    if (!init_success) {
        Serial.println("Viable sensor BMP280 not found, check wiring!");
    }
}

void get_bmp([[maybe_unused]] void *pvParameters) {
    for (;;) {
        if (init_success) {
            data = Data{.temperature = bmp_obj.readTemperature(),
                        .pressure = bmp_obj.readPressure(),
                        .altitude = bmp_obj.readAltitude()};
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void print_data([[maybe_unused]] void *pvParameters) {
    for (;;) {
        pretty_print();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void pretty_print() {
    Serial.print("[Temperature] ");
    Serial.print(data.temperature);
    Serial.println("°C");

    Serial.print("[Pressure] ");
    Serial.print(data.hpa());
    Serial.println("hPa");

    Serial.print("[Altitude] ");
    Serial.print(data.altitude);
    Serial.println("m");
}

Data get_data() { return data; }

} // namespace bmp
