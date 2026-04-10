#include "bme280.h"
#include "common.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "i2c.h"
#include "lsm6dso.h"
#include "storage.h"

constexpr inline const char *TAG = "ROCKET";

// delay between sensor reads (in milliseconds)
static constexpr inline int MAIN_TICK_INTERVAL = 50;

static void mainLoopTimerCallback(void *arg);
static void sensorReadTask(void *pvParameters);

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Initializing ROCKET");

    vTaskDelay(500 / portTICK_PERIOD_MS);

    if (!mainSemaphoreInit().has_value()) {
        return;
    }

    if (i2c::init().has_value()) {
        if (auto res = bme280::init(); !res.has_value()) {
            ESP_LOGE(TAG,
                     "Initializing BME280 failed (CODE %d). Proceeding anyways",
                     res.error());
        }
        if (auto res = lsm6dso::init(); !res.has_value()) {
            ESP_LOGE(
                TAG,
                "Initializing LSM6DSO failed (CODE %d). Proceeding anyways",
                res.error());
        }
    } else {
        ESP_LOGE(TAG, "Initializing I2C failed. Proceeding anyways");
    }

    esp_timer_handle_t timer{};
    auto sensor_read_semaphore = xSemaphoreCreateBinary();
    if (sensor_read_semaphore == nullptr) {
        ESP_LOGE(TAG, "Failed to create main loop semaphore. Exiting");
        return;
    }
    if (xTaskCreatePinnedToCore(sensorReadTask, "SENSOR_READ_TASK",
                                /* usStackDepth = */ 4096 * 16,
                                sensor_read_semaphore,
                                /* uxPriority = */ 15, nullptr, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create sensor read RTOS task");
        while (true) {
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    } else {
        const esp_timer_create_args_t timer_config = {
            .callback = mainLoopTimerCallback,
            .arg = sensor_read_semaphore,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "SENSOR_READ_TIMER",
            .skip_unhandled_events = true};

        if (auto res = esp_timer_create(&timer_config, &timer); res != ESP_OK) {
            ESP_LOGE(TAG,
                     "Main loop timer creation failed (code: %d). Proceeding "
                     "anyways",
                     res);
        }
    }

    if (not storage::init().has_value()) {
        ESP_LOGE(TAG, "Initializing STORAGE failed. Proceeding anyways");
    }

    if (not storage::start().has_value()) {
        ESP_LOGE(TAG, "Running STORAGE failed. Proceeding anyways");
    }

    if (auto res =
            esp_timer_start_periodic(timer, MAIN_TICK_INTERVAL * 1000LLU);
        res != ESP_OK) {
        ESP_LOGE(TAG,
                 "Main loop timer start failed (code: %d). Proceeding anyways",
                 res);
    }

    while (true) {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

static void mainLoopTimerCallback(void *arg) {
    auto sensor_read_semaphore = static_cast<SemaphoreHandle_t>(arg);
    xSemaphoreGive(sensor_read_semaphore);
}

static void sensorReadTask(void *pvParameters) {
    auto semaphore = static_cast<SemaphoreHandle_t>(pvParameters);
    while (true) {
        static int64_t last_time = 0;
        if (last_time == 0) {
            last_time = esp_timer_get_time();
        }

        if (xSemaphoreTake(semaphore, 500) == pdTRUE) {
            // mainSemaphoreTake();

            auto time = esp_timer_get_time();
            auto time_diff = time - last_time;
            last_time = time;

            auto res = bme280::readAndProcessData();
            if (!res.has_value()) {
                ESP_LOGE(TAG, "Reading BME280 data failed (CODE %d)",
                         res.error());
            }
            res = lsm6dso::readAndProcessData();
            if (!res.has_value()) {
                ESP_LOGE(TAG, "Reading LSM6DSO data failed (CODE %d)",
                         res.error());
            }

            // mainSemaphoreGive();

            ESP_LOGI(TAG,
                     "Sensor data has been read.\tSince last read: "
                     "%2.2fms\tReading data took: %.2fms",
                     time_diff / 1000.0f,
                     (esp_timer_get_time() - time) / 1000.0f);
        }
    }
}
