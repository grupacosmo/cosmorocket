#include "storage.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <ring_buffer.h>
#include <sys/stat.h>

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <fstream>
#include <string>
#include <tuple>

#include "common.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_timer.h"

namespace storage {

constexpr inline const char *TAG = "STORAGE";

constexpr const char *PARTITION_LABEL = "littlefs";
constexpr const char *PARTITION_PATH = "/littlefs/";
constexpr const char *PRESSURE_TEMP_FILENAME = "PRESSURE_TEMP.BIN";
constexpr const char *ACCELERATION_FILENAME = "ACCELERATION.BIN";
constexpr const char *ANGULAR_RATE_FILENAME = "ANGULAR_RATE.BIN";

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

static auto openFile(const char *filename)
    -> std::expected<std::ofstream, Error> {
    std::ofstream file(std::string(PARTITION_PATH) + filename,
                       std::ios::out | std::ios::app | std::ios::binary);
    if (!file.good()) {
        return std::unexpected(Error::STORAGE_FILE_OPENING_FAILED);
    }
    return {std::move(file)};
}

static auto flushPressureTemperature() -> std::expected<Success, Error> {
    // ESP_LOGI(TAG, "BME280 data:");

    auto file = openFile(PRESSURE_TEMP_FILENAME);
    if (!file.has_value()) {
        return std::unexpected(file.error());
    }

    while (!g_barometer_data_buffer.empty()) {
        bme280::Data data{};
        g_barometer_data_buffer.pop(data);

        if (!file.value().write(reinterpret_cast<const char *>(&data),
                                sizeof(data))) {
            ESP_LOGE(TAG, "Error writing to file: (code: %d)",
                     file.value().exceptions());
            return std::unexpected(Error::STORAGE_FILE_WRITE_FAILED);
        }

        // ESP_LOGI(TAG, "%6.2fPa, %2.2f%%, %2.2fC", data.air_pressure,
        //          data.humidity, data.temperature);
    }
    file->close();

    return Success{};
}

static auto flushAcceleration() -> std::expected<Success, Error> {
    // ESP_LOGI(TAG, "LSM6DSO32 acceleration:");

    auto file = openFile(ACCELERATION_FILENAME);
    if (!file.has_value()) {
        return std::unexpected(file.error());
    }

    int cnt = 0;
    while (!g_acceleration_buffer.empty()) {
        std::array<int16_t, 3> data{};
        g_acceleration_buffer.pop(data);

        if (!file.value().write(reinterpret_cast<const char *>(data.data()),
                                sizeof(int16_t) * 3)) {
            ESP_LOGE(TAG, "Error writing to file (code: %d)",
                     file.value().exceptions());
            return std::unexpected(Error::STORAGE_FILE_WRITE_FAILED);
        }

        // if (cnt < 5)
        //     ESP_LOGI(TAG, "X: %6d Y: %6d Z: %6d", data[0], data[1], data[2]);

        cnt++;
    }
    // ESP_LOGI(TAG, "And %d more records", cnt - 5);
    file->close();

    return Success{};
}

static auto flushAngularRate() -> std::expected<Success, Error> {
    // ESP_LOGI(TAG, "LSM6DSO32 angular rate:");

    auto file = openFile(ANGULAR_RATE_FILENAME);
    if (!file.has_value()) {
        return std::unexpected(file.error());
    }

    int cnt = 0;
    while (!g_angular_rate_buffer.empty()) {
        std::array<int16_t, 3> data{};
        g_angular_rate_buffer.pop(data);

        if (!file.value().write(reinterpret_cast<const char *>(data.data()),
                                sizeof(int16_t) * 3)) {
            ESP_LOGE(TAG, "Error writing to file (code: %d)",
                     file.value().exceptions());
            return std::unexpected(Error::STORAGE_FILE_WRITE_FAILED);
        }

        // if (cnt < 5)
        //     ESP_LOGI(TAG, "X: %6d Y: %6d Z: %6d", data[0], data[1], data[2]);

        cnt++;
    }
    // ESP_LOGI(TAG, "And %d more records", cnt - 5);
    file->close();

    return Success{};
}

static void flushTask(void *pvParameters) {
    auto sem = static_cast<SemaphoreHandle_t>(pvParameters);

    while (true) {
        if (xSemaphoreTake(sem, 500) == pdTRUE) {
            // ESP_LOGI(TAG, "Storage flush started");

            std::ignore = flushPressureTemperature();
            std::ignore = flushAcceleration();
            std::ignore = flushAngularRate();

            // ESP_LOGI(TAG, "Storage flush complete");

            size_t storage = 0;
            size_t used = 0;
            if (auto ret = esp_littlefs_info(PARTITION_LABEL, &storage, &used);
                ret != ESP_OK) {
                ESP_LOGW(TAG,
                         "Failed to get info about available LittleFS storage "
                         "(%s). ",
                         esp_err_to_name(ret));
            } else {
                ESP_LOGI(TAG, "Storage used: %d, free: %d", used,
                         storage - used);
            }
        }
    }
}

static auto createEmptyFile(const char *filename)
    -> std::expected<Success, Error> {
    std::ofstream file(std::string(PARTITION_PATH) + filename,
                       std::ios::out | std::ios::binary);
    if (!file.good()) {
        ESP_LOGE(TAG, "Could not create file (error bits: %d)",
                 file.exceptions());
        return std::unexpected(Error::STORAGE_FILE_CREATION_FAILED);
    }
    file.close();
    return Success{};
}

static auto initFilesystem() -> std::expected<Success, Error> {
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = PARTITION_LABEL,
        .format_if_mount_failed = 1,
        .dont_mount = 0,
    };

    uint32_t size_flash_chip{};
    if (auto ret = esp_flash_get_size(NULL, &size_flash_chip); ret != ESP_OK) {
        ESP_LOGW(TAG, "Could not get flash size (code: %d). Proceeding anyways",
                 ret);
    }

    ESP_LOGI(TAG, "Initializing storage. Flash memory size: %d",
             size_flash_chip);

    if (auto ret = esp_vfs_littlefs_register(&conf); ret != ESP_OK) {
        ESP_LOGE(TAG, "Could not mount LittleFS filesystem (code: %d)", ret);
        return std::unexpected(Error::STORAGE_INIT_FAILED);
    }

    // Save empty data files
    return createEmptyFile(PRESSURE_TEMP_FILENAME)
        .and_then([](Success const &) {
            return createEmptyFile(ACCELERATION_FILENAME)
                .and_then([](Success const &) {
                    return createEmptyFile(ANGULAR_RATE_FILENAME);
                });
        });
}

auto init() -> std::expected<Success, Error> {
    return initFilesystem().and_then(
        [](Success const &) -> std::expected<Success, Error> {
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
            esp_timer_handle_t timer{};
            if (auto res = esp_timer_create(&timer_config, &timer);
                res != ESP_OK) {
                ESP_LOGE(TAG, "ESP timer creation failed (code: %d)", res);
                return std::unexpected(Error::STORAGE_INIT_FAILED);
            }
            if (auto res = esp_timer_start_periodic(
                    timer, DATA_FLUSH_INTERVAL * 1000LLU);
                res != ESP_OK) {
                ESP_LOGE(TAG, "ESP timer start failed (code: %d)", res);
                return std::unexpected(Error::STORAGE_INIT_FAILED);
            }

            if (xTaskCreate(flushTask, "STORAGE_FLUSH_TASK",
                            /* usStackDepth = */ 40960, sem,
                            /* uxPriority = */ 1, nullptr) != pdPASS) {
                ESP_LOGE(TAG, "Failed to create RTOS task");
                return std::unexpected(Error::STORAGE_INIT_FAILED);
            }

            return Success{};
        });
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