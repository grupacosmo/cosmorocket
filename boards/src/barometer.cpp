#include "barometer.h"

#include <Adafruit_BMP280.h>

namespace barometer {

Adafruit_BMP280 g_sensor;

void init() {
    bool success =
        g_sensor.begin(0x76, 0x60) ||  // Primary address, primary chip ID
        g_sensor.begin(0x76, 0x58) ||  // Primary address, alternative chip ID
        g_sensor.begin(0x77, 0x60) ||  // Alternative address, primary chip ID
        g_sensor.begin(0x77, 0x58);  // Alternative address, alternative chip ID

    if (!success) {
        Serial.println("BMP280 not found.");
        return;
    }

    g_sensor.setSampling(
        Adafruit_BMP280::MODE_FORCED,     // Operating Mode
        Adafruit_BMP280::SAMPLING_X2,     // Temperature oversampling
        Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
        Adafruit_BMP280::FILTER_X16,      // Filtering
        Adafruit_BMP280::STANDBY_MS_63);  // Standby time
}

void measure() {
    if (!g_sensor.takeForcedMeasurement()) {
        Serial.println("BMP280 measurement failed. Reinitialising...");
        init();
    }
}

Data::Temperature getTemperature() {
    return Data::Temperature{g_sensor.readTemperature()};
}

Data::Pressure getPressure() { return Data::Pressure{g_sensor.readPressure()}; }

}  // namespace barometer
