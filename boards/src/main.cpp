#include <HardwareSerial.h>

#include "accelerometer.h"
#include "barometer.h"
#include "board_config.h"
#include "i2c.h"

// Fine-tuned to provide ~51ms delay between measurements
static constexpr inline int MEASUREMENT_DELAY = 17;

void setup() {
    Serial.begin(115200);
    Serial.println("Rocket initialisation started");

    i2c::init();
    barometer::init();
    accelerometer::init();
}

void loop() {
    static int64_t last_ticks = 0;
    auto ticks = esp_timer_get_time();
    if (last_ticks == 0) last_ticks = ticks;
    auto tick_diff = ticks - last_ticks;
    last_ticks = ticks;

    Serial.printf("Time passed: %.2f   ", tick_diff / 1000.0f);

    vTaskDelay(MEASUREMENT_DELAY / portTICK_PERIOD_MS);

    barometer::Pressure air_pressure;
    barometer::Temperature temperature;
    barometer::getData(air_pressure, temperature);

    accelerometer::readData();
    size_t acceleration_cnt = accelerometer::getAccelelerationCount();
    size_t angular_rate_cnt = accelerometer::getAngularRateCount();

    // Get last acceleration measurement
    auto &acceleration = (accelerometer::getAccelerationBuffer())[acceleration_cnt - 1];

    // Get last angular rate measurement
    auto &angular_rate = (accelerometer::getAngularRateBuffer())[angular_rate_cnt - 1];

    Serial.printf("%.2fPa, %.2fC    Accel: %d %d %d    Angular rate: %d %d %d\n", air_pressure,
                  temperature, acceleration[0], acceleration[1], acceleration[2], angular_rate[0],
                  angular_rate[1], angular_rate[2]);
}
