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
#include <string>
#include <tuple>
#include <vector>

#include "common.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"

namespace storage {

constexpr inline const char *TAG = "STORAGE";

constexpr const char *PARTITION_LABEL = "littlefs";
constexpr const char *PARTITION_PATH = "/littlefs/";
constexpr const char *PRESSURE_TEMP_FILENAME = "PRESSURE_TEMP.BIN";
constexpr const char *ACCELERATION_FILENAME = "ACCELERATION.BIN";
constexpr const char *ANGULAR_RATE_FILENAME = "ANGULAR_RATE.BIN";

// 1000ms delay between data flushes
static constexpr inline int DATA_FLUSH_INTERVAL = 4000;

// Thread safe (atomic) Single Producer Single Consumer ring buffer
SPSCQueue<bme280::Data, 128> g_barometer_data_buffer;
SPSCQueue<std::array<int16_t, 3>, 2048> g_acceleration_buffer;
SPSCQueue<std::array<int16_t, 3>, 2048> g_angular_rate_buffer;
// SPSCQueue<gps::Data, 64> g_gps_data_buffer;

FILE *g_pressure_temp_file;
FILE *g_acceleration_file;
FILE *g_angular_rate_file;

esp_timer_handle_t g_timer{};

static void timerCallback(void *arg) {
    auto sem = static_cast<SemaphoreHandle_t>(arg);
    xSemaphoreGive(sem);
}

static auto openFile(const char *filename) -> std::expected<FILE *, Error> {
    auto file = fopen((std::string(PARTITION_PATH) + filename).c_str(), "wb");
    if (file == nullptr) {
        return std::unexpected(Error::STORAGE_FILE_OPENING_FAILED);
    }
    return file;
}

auto flushPressureTemperature() -> std::expected<Success, Error> {
    // ESP_LOGI(TAG, "BME280 data:");

    if (g_barometer_data_buffer.empty()) {
        return Success{};
    }

    std::vector<bme280::Data> tmp_buffer{};

    while (not g_barometer_data_buffer.empty()) {
        bme280::Data data{};
        g_barometer_data_buffer.pop(data);

        tmp_buffer.push_back(data);

        ESP_LOGI(TAG, "%6.2fPa, %2.2f%%, %2.2fC", data.air_pressure,
                 data.humidity, data.temperature);
    }

    if (tmp_buffer.empty()) {
        return Success{};
    }

    // mainSemaphoreTake();
    if (fwrite(reinterpret_cast<const char *>(&tmp_buffer.at(0)),
               sizeof(bme280::Data), tmp_buffer.size(),
               g_pressure_temp_file) == 0) {
        ESP_LOGE(TAG, "Error writing to file");
        return std::unexpected(Error::STORAGE_FILE_WRITE_FAILED);
    }

    if (fsync(fileno(g_pressure_temp_file)) == -1) {
        ESP_LOGE(TAG, "Could not flush file");
        return std::unexpected(Error::STORAGE_FILE_WRITE_FAILED);
    }
    // mainSemaphoreGive();

    return Success{};
}

auto flushAcceleration() -> std::expected<Success, Error> {
    // ESP_LOGI(TAG, "LSM6DSO32 acceleration:");

    if (g_acceleration_buffer.empty()) {
        return Success{};
    }

    std::vector<std::array<int16_t, 3>> tmp_buffer{};
    tmp_buffer.reserve(500);

    int cnt = 0;
    while (not g_acceleration_buffer.empty()) {
        std::array<int16_t, 3> data{};
        g_acceleration_buffer.pop(data);

        tmp_buffer.push_back(data);

        if (cnt < 5)
            ESP_LOGI(TAG, "Acceleration: X: %6d Y: %6d Z: %6d", data[0],
                     data[1], data[2]);

        cnt++;
    }
    // ESP_LOGI(TAG, "And %d more records", cnt - 5);

    if (tmp_buffer.empty()) {
        return Success{};
    }

    // mainSemaphoreTake();
    if (fwrite(reinterpret_cast<const char *>(&tmp_buffer.at(0)),
               sizeof(std::array<int16_t, 3>), tmp_buffer.size(),
               g_acceleration_file) == 0) {
        ESP_LOGE(TAG, "Error writing to file");
        return std::unexpected(Error::STORAGE_FILE_WRITE_FAILED);
    }

    if (fsync(fileno(g_acceleration_file)) == -1) {
        ESP_LOGE(TAG, "Could not flush file");
        return std::unexpected(Error::STORAGE_FILE_WRITE_FAILED);
    }
    // mainSemaphoreGive();

    return Success{};
}

auto flushAngularRate() -> std::expected<Success, Error> {
    // ESP_LOGI(TAG, "LSM6DSO32 angular rate:");

    if (g_angular_rate_buffer.empty()) {
        return Success{};
    }

    std::vector<std::array<int16_t, 3>> tmp_buffer{};
    tmp_buffer.reserve(500);

    int cnt = 0;
    while (not g_angular_rate_buffer.empty()) {
        std::array<int16_t, 3> data{};
        g_angular_rate_buffer.pop(data);

        tmp_buffer.push_back(data);

        if (cnt < 5)
            ESP_LOGI(TAG, "Angular rate: X: %6d Y: %6d Z: %6d", data[0],
                     data[1], data[2]);

        cnt++;
    }
    // ESP_LOGI(TAG, "And %d more records", cnt - 5);

    if (tmp_buffer.empty()) {
        return Success{};
    }

    // mainSemaphoreTake();
    if (fwrite(reinterpret_cast<const char *>(&tmp_buffer.at(0)),
               sizeof(std::array<int16_t, 3>), tmp_buffer.size(),
               g_angular_rate_file) == 0) {
        ESP_LOGE(TAG, "Error writing to file");
        return std::unexpected(Error::STORAGE_FILE_WRITE_FAILED);
    }

    if (fsync(fileno(g_angular_rate_file)) == -1) {
        ESP_LOGE(TAG, "Could not flush file");
        return std::unexpected(Error::STORAGE_FILE_WRITE_FAILED);
    }
    // mainSemaphoreGive();

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

static auto initFilesystem() -> std::expected<Success, Error> {
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = PARTITION_LABEL,
        .partition = nullptr,
        .format_if_mount_failed = 1,
        .dont_mount = 0,
    };

    uint32_t size_flash_chip{};
    if (auto ret = esp_flash_get_size(nullptr, &size_flash_chip);
        ret != ESP_OK) {
        ESP_LOGW(TAG, "Could not get flash size (code: %d). Proceeding anyways",
                 ret);
    }

    ESP_LOGI(TAG, "Initializing storage. Flash memory size: %d",
             size_flash_chip);

    if (auto ret = esp_vfs_littlefs_register(&conf); ret != ESP_OK) {
        ESP_LOGE(TAG, "Could not mount LittleFS filesystem (code: %d)", ret);
        return std::unexpected(Error::STORAGE_INIT_FAILED);
    }

    // Create empty data files
    auto res = openFile(PRESSURE_TEMP_FILENAME);
    if (!res.has_value()) {
        return std::unexpected(res.error());
    }
    g_pressure_temp_file = res.value();

    res = openFile(ACCELERATION_FILENAME);
    if (!res.has_value()) {
        return std::unexpected(res.error());
    }
    g_acceleration_file = res.value();

    res = openFile(ANGULAR_RATE_FILENAME);
    if (!res.has_value()) {
        return std::unexpected(res.error());
    }
    g_angular_rate_file = res.value();

    return Success{};
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

            if (auto res = esp_timer_create(&timer_config, &g_timer);
                res != ESP_OK) {
                ESP_LOGE(TAG, "ESP timer creation failed (code: %d)", res);
                return std::unexpected(Error::STORAGE_INIT_FAILED);
            }

            if (xTaskCreatePinnedToCore(flushTask, "STORAGE_FLUSH_TASK",
                                        /* usStackDepth = */ 4096 * 16, sem,
                                        /* uxPriority = */ 10, nullptr,
                                        1) != pdPASS) {
                ESP_LOGE(TAG, "Failed to create RTOS task");
                return std::unexpected(Error::STORAGE_INIT_FAILED);
            }

            return Success{};
        });
}

auto start() -> std::expected<Success, Error> {
    if (auto res =
            esp_timer_start_periodic(g_timer, DATA_FLUSH_INTERVAL * 1000LLU);
        res != ESP_OK) {
        ESP_LOGE(TAG, "ESP timer start failed (code: %d)", res);
        return std::unexpected(Error::STORAGE_INIT_FAILED);
    }

    return Success{};
}

void postBarometerData(const bme280::Data &data) {
    if (not g_barometer_data_buffer.push(data))
        ESP_LOGE(TAG, "Storage barometer data buffer overflow");
}

void postAccelerationData(const std::array<int16_t, 3> &data) {
    if (not g_acceleration_buffer.push(data))
        ESP_LOGE(TAG, "Storage acceleration buffer overflow");
}

void postAngularRateData(const std::array<int16_t, 3> &data) {
    if (not g_angular_rate_buffer.push(data))
        ESP_LOGE(TAG, "Storage angular rate buffer overflow");
}

// void postGpsData(const gps::Data &data) {
//     if (not g_gps_data_buffer.push(data))
//         ESP_LOGE(TAG, "GPS data buffer overflow");
// }

}  // namespace storage