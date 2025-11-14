#include <HardwareSerial.h>

#include "accelerometer.h"
#include "barometer.h"
#include "board_config.h"
#include "gps.h"
#include "i2c.h"
#include "storage.h"

// 50ms delay between reports
static constexpr inline int MAIN_TICK_INTERVAL = 50;

// In miliseconds
static constexpr inline int SERIAL_WAIT_POLL_INTERVAL = 10;

// Semaphore used to inform the main loop that the tick interval has passed
SemaphoreHandle_t g_main_loop_semaphore;
esp_timer_handle_t g_main_loop_timer;

static void main_loop_timer_callback(void *arg);

void setup() {
    Serial.begin(115200);

    if (board_config::WAIT_FOR_SERIAL)
        while (!Serial) vTaskDelay(SERIAL_WAIT_POLL_INTERVAL / portTICK_PERIOD_MS);

    Serial.println("Rocket initialisation started");

    gps::init();
    storage::init();
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
    static int64_t last_time = 0;
    if (last_time == 0) last_time = esp_timer_get_time();

    // Wait for the timer to measure given interval
    if (xSemaphoreTake(g_main_loop_semaphore, 500) == pdTRUE) {
        auto time = esp_timer_get_time();
        auto time_diff = time - last_time;
        last_time = time;

        barometer::readAndProcessData();
        accelerometer::readData();

        Serial.printf(
            "Sensor read task:    Since last read: %2.2fms    Reading data took: %.2fms\n",
            time_diff / 1000.0f, (esp_timer_get_time() - time) / 1000.0f);
    }
}

static void main_loop_timer_callback(void *arg) { xSemaphoreGive(g_main_loop_semaphore); }
