#include <HardwareSerial.h>

#include "accelerometer.h"
#include "barometer.h"
#include "board_config.h"
#include "i2c.h"

// Fine-tuned to provide 50ms delay between measurements
static constexpr inline int MAIN_TICK_INTERVAL = 50;

// Semaphore used to inform the main loop that the tick interval has passed
SemaphoreHandle_t g_main_loop_semaphore;
esp_timer_handle_t g_main_loop_timer;

static void main_loop_timer_callback(void *arg);

void setup() {
    Serial.begin(115200);
    Serial.println("Rocket initialisation started");

    i2c::init();
    barometer::init();
    accelerometer::init();

    const esp_timer_create_args_t timer_config = {.callback = main_loop_timer_callback,
                                                  .arg = nullptr,
                                                  .dispatch_method = ESP_TIMER_TASK,
                                                  .name = "main-loop-timer",
                                                  .skip_unhandled_events = true};
    esp_timer_create(&timer_config, &g_main_loop_timer);
    esp_timer_start_periodic(g_main_loop_timer, MAIN_TICK_INTERVAL * 1000);

    g_main_loop_semaphore = xSemaphoreCreateBinary();

    Serial.println("Rocket initialisation completed");
}

void loop() {
    if (xSemaphoreTake(g_main_loop_semaphore, 500) == pdTRUE) {
        static int64_t last_time = 0;
        auto time = esp_timer_get_time();
        if (last_time == 0) last_time = time;
        auto time_diff = time - last_time;
        last_time = time;

        Serial.printf("Time passed: %.2f   ", time_diff / 1000.0f);

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
                      temperature, acceleration[0], acceleration[1], acceleration[2],
                      angular_rate[0], angular_rate[1], angular_rate[2]);
    }
}

static void main_loop_timer_callback(void *arg) { xSemaphoreGive(g_main_loop_semaphore); }
