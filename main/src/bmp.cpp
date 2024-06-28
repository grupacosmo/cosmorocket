#include "bmp.h"

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

// 1019.91 is avg Pressure in Cracow
#define SEALEVELPRESSURE_HPA (1019.91)
#define CHIP_BME 0x60
#define CHIP_BMP 0x58
#define CHIP_ADDR 0x76

namespace bmp {

Adafruit_BMP280 bmp_obj;

// Requires Wire.begin(SDA_PIN, SCL_PIN)
void init() {
    if (!bmp_obj.begin(CHIP_ADDR, CHIP_BME)) {
        Serial.println("Viable sensor BMP280 not found, check wiring!");
        for (;;) {
        }
    }
}

Data measurements() {
    Data check;
    check.temperature = bmp_obj.readTemperature();
    check.pressure = bmp_obj.readPressure();
    check.altitude = bmp_obj.readAltitude(SEALEVELPRESSURE_HPA);
    return check;
};

void get_bmp(void *pvParameters) {
    for (;;) {
        data = measurements();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
void print_data(void *pvParameters) {
    for (;;) {
        pretty_print(data);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void pretty_print(Data data) {
    Serial.print("Temperature = ");
    Serial.print(data.temperature);
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(data.get_as_hpa());
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(data.altitude);
    Serial.println(" m");

    Serial.println();
}

float Data::get_as_hpa() { return bmp_obj.readPressure() / 100.0F; }
} // namespace bmp
