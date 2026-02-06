#include "bme280.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "i2c.h"
#include "lsm6dso.h"

constexpr inline const char *TAG = "ROCKET";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "INITIALIZING ROCKET");

    if (i2c::init().has_value()) {
        auto res = bme280::init();
        if (!res.has_value()) {
            ESP_LOGE(TAG,
                     "ERROR INITIALIZING BME280 (CODE %d). PROCEEDING ANYWAYS",
                     res.error());
        }
        res = lsm6dso::init();
        if (!res.has_value()) {
            ESP_LOGE(TAG,
                     "ERROR INITIALIZING LSM6DSO (CODE %d). PROCEEDING ANYWAYS",
                     res.error());
        }
    } else {
        ESP_LOGE(TAG, "ERROR INITIALIZING I2C. PROCEEDING ANYWAYS");
    }

    while (true) {
        auto res = bme280::readAndProcessData();
        if (!res.has_value()) {
            ESP_LOGE(TAG, "ERROR READING BME280 DATA (CODE %d)", res.error());
        }
        res = lsm6dso::readAndProcessData();
        if (!res.has_value()) {
            ESP_LOGE(TAG, "ERROR READING LSM6DSO DATA (CODE %d)", res.error());
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
