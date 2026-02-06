#include "storage.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <ring_buffer.h>

#include <cstddef>
#include <expected>

#include "common.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

namespace storage {

constexpr inline const char *TAG = "STORAGE";

// 1000ms delay between data flushes
static constexpr inline int DATA_FLUSH_INTERVAL = 1000;

// Thread safe (atomic) Single Producer Single Consumer ring buffer
SPSCQueue<bme280::Data, 32> g_barometer_data_buffer;
SPSCQueue<std::array<int16_t, 3>, 1024> g_acceleration_buffer;
SPSCQueue<std::array<int16_t, 3>, 1024> g_angular_rate_buffer;
// SPSCQueue<gps::Data, 64> g_gps_data_buffer;

static void timerCallback(void *arg) {
    auto sem = static_cast<SemaphoreHandle_t>(arg);
    xSemaphoreGive(sem);
}

static void flushTask(void *pvParameters) {
    auto sem = static_cast<SemaphoreHandle_t>(pvParameters);

    while (true) {
        if (xSemaphoreTake(sem, 500) == pdTRUE) {
            ESP_LOGI(TAG, "Storage flush started");

            ESP_LOGI(TAG, "BME280 data:");
            while (!g_barometer_data_buffer.empty()) {
                bme280::Data data{};
                g_barometer_data_buffer.pop(data);

                ESP_LOGI(TAG, "%6.2fPa, %2.2f%%, %2.2fC", data.air_pressure,
                         data.humidity, data.temperature);
            }

            ESP_LOGI(TAG, "LSM6DSO32 acceleration:");
            int cnt = 0;
            while (!g_acceleration_buffer.empty()) {
                std::array<int16_t, 3> data{};
                g_acceleration_buffer.pop(data);

                if (cnt < 5)
                    ESP_LOGI(TAG, "X: %6d Y: %6d Z: %6d", data[0], data[1],
                             data[2]);

                cnt++;
            }
            ESP_LOGI(TAG, "And %d more records", cnt - 5);

            ESP_LOGI(TAG, "LSM6DSO32 angular rate:");
            cnt = 0;
            while (!g_angular_rate_buffer.empty()) {
                std::array<int16_t, 3> data{};
                g_angular_rate_buffer.pop(data);

                if (cnt < 5)
                    ESP_LOGI(TAG, "X: %6d Y: %6d Z: %6d", data[0], data[1],
                             data[2]);

                cnt++;
            }
            ESP_LOGI(TAG, "And %d more records", cnt - 5);

            // ESP_LOGI(TAG, "GPS data:");
            // while (!g_gps_data_buffer.empty()) {
            //     gps::Data data;
            //     g_gps_data_buffer.pop(data);

            //     ESP_LOGI(TAG, "%s\n", data.data());
            // }

            ESP_LOGI(TAG, "Storage flush complete");
        }
    }
}

auto init() -> Result<Success> {
    auto sem = xSemaphoreCreateBinary();
    if (sem == nullptr) {
        ESP_LOGE(TAG, "Semaphore creation error");
        return std::unexpected(Error::STORAGE_INIT_FAILED);
    }

    const esp_timer_create_args_t timer_config = {
        .callback = timerCallback,
        .arg = sem,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "STORAGE_FLUSH_TIMER",
        .skip_unhandled_events = true};
    esp_timer_handle_t timer;
    if (auto res = esp_timer_create(&timer_config, &timer); res != ESP_OK) {
        ESP_LOGE(TAG, "ESP timer creation failed (code: %d)", res);
        return std::unexpected(Error::STORAGE_INIT_FAILED);
    }
    if (auto res =
            esp_timer_start_periodic(timer, DATA_FLUSH_INTERVAL * 1000LLU);
        res != ESP_OK) {
        ESP_LOGE(TAG, "ESP timer start failed (code: %d)", res);
        return std::unexpected(Error::STORAGE_INIT_FAILED);
    }

    if (xTaskCreate(flushTask, "STORAGE_FLUSH_TASK", /* ucStackDepth = */ 4096,
                    sem, /* uxPriority = */ 11, nullptr) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create RTOS task");
        return std::unexpected(Error::STORAGE_INIT_FAILED);
    }

    return Success{};
}

void postBarometerData(const bme280::Data &data) {
    if (!g_barometer_data_buffer.push(data))
        ESP_LOGE(TAG, "Storage barometer data buffer overflow");
}

void postAccelerationData(const std::array<int16_t, 3> &data) {
    if (!g_acceleration_buffer.push(data))
        ESP_LOGE(TAG, "Storage acceleration buffer overflow");
}

void postAngularRateData(const std::array<int16_t, 3> &data) {
    if (!g_angular_rate_buffer.push(data))
        ESP_LOGE(TAG, "Storage angular rate buffer overflow");
}

// void postGpsData(const gps::Data &data) {
//     if (!g_gps_data_buffer.push(data))
//         ESP_LOGE(TAG, "GPS data buffer overflow");
// }

}  // namespace storage