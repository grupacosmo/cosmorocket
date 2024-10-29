#include "bmp.h"

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

namespace bmp {

// Private
namespace {
    // 1019.91 is avg Pressure in Cracow
    constexpr float SEALEVELPRESSURE_HPA = 1019.91;

    constexpr std::uint8_t CHIP_BME = 0x60;  // ALT 0x58
    constexpr std::uint8_t CHIP_ADDR = 0x76; // ALT 0x77

    Adafruit_BMP280 bmp_obj;

    void print_debug(const Data& data) {
        Serial.print("[Temperature] ");
        Serial.print(data.temperature);
        Serial.println("°C");

        Serial.print("[Pressure] ");
        Serial.print(data.pressure / 100.f);
        Serial.println("hPa");

        Serial.print("[Altitude] ");
        Serial.print(data.altitude);
        Serial.println("m");
    }
} // namespace

void init() {
    bool success = bmp_obj.begin(CHIP_ADDR, CHIP_BME);

    if (!success) {
        Serial.println("Viable sensor BMP280 not found, check wiring!");
    }
}

void get_bmp(void *pvParameters) {
    for (;;) {
        if (bmp_obj.sensorID != 0) { // TODO Test should be 0 if not inited
            data = Data{.temperature = bmp_obj.readTemperature(),
                        .pressure = bmp_obj.readPressure(),
                        .altitude = bmp_obj.readAltitude(SEALEVELPRESSURE_HPA)};
#ifdef DEBUG
            print_debug(&data);
#endif
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

} // namespace bmp
