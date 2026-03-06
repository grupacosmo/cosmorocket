#include "bme280.h"

#include <unistd.h>

#include <ctime>
#include <expected>
#include <functional>
#include <utility>
#include <variant>

#include "BME280_SensorAPI/bme280.h"
#include "common.h"
#include "esp_log.h"
#include "i2c.h"
#include "sdkconfig.h"
#include "storage.h"

namespace bme280 {

constexpr inline const char *TAG = "BME280";

// Indicates that an error has occurred during initialization and all subsequent
// operations will fail.
bool g_initialized = false;
// Sensor driver context
struct bme280_dev g_sensor;

// I/O functions passed to bme280 driver
BME280_INTF_RET_TYPE platform_read(uint8_t reg_addr, uint8_t *reg_data,
                                   uint32_t length, void *intf_ptr) {
    return i2c::read(CONFIG_BME280_I2C_ADDR, reg_addr, reg_data, length)
        .transform([](Success) -> BME280_INTF_RET_TYPE { return 0; })
        .value_or(-1);
}

BME280_INTF_RET_TYPE platform_write(uint8_t reg_addr, const uint8_t *reg_data,
                                    uint32_t length, void *intf_ptr) {
    return i2c::write(CONFIG_BME280_I2C_ADDR, reg_addr, reg_data, length)
        .transform([](Success) -> BME280_INTF_RET_TYPE { return 0; })
        .value_or(-1);
}

void bme280_delay_us(uint32_t period, void *intf_ptr) { usleep(period); }

auto init() -> std::expected<Success, Error> {
    g_sensor.intf = BME280_I2C_INTF;
    g_sensor.intf_ptr = nullptr;  // Not used
    g_sensor.read = platform_read;
    g_sensor.write = platform_write;
    g_sensor.delay_us = bme280_delay_us;

    int8_t res = bme280_init(&g_sensor);
    if (res != BME280_OK) {
        ESP_LOGE(TAG, "sensor not found. %d", res);
        return std::unexpected(Error::BME280_INIT_FAILED);
    }

    struct bme280_settings settings;

    // Filter is of no use to us, because we need to know values at the moment,
    // even if they reach extremes and last only a short while
    settings.filter = BME280_FILTER_COEFF_OFF;

    // Only the pressure is oversampled to provide higher accuracy
    settings.osr_h = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_16X;
    settings.osr_t = BME280_OVERSAMPLING_1X;

    // Measure continuously
    settings.standby_time = BME280_STANDBY_TIME_0_5_MS;

    res = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings,
                                     &g_sensor);
    if (res != BME280_OK) {
        ESP_LOGE(TAG, "Set settings failed");
        return std::unexpected(Error::BME280_INIT_FAILED);
    }

    res = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &g_sensor);
    if (res != BME280_OK) {
        ESP_LOGE(TAG, "Set mode failed");
        return std::unexpected(Error::BME280_INIT_FAILED);
    }

    uint32_t period = 0;
    bme280_cal_meas_delay(&period, &settings);
    ESP_LOGD(TAG, "Measurement time [ms]: %f", period / 1000.0f);

    g_initialized = true;

    return Success{};
}

static auto readData() -> std::expected<Data, Error> {
    Data data{};
    if (not g_initialized) {
        return std::unexpected(Error::BME280_INIT_FAILED);
    }

    struct bme280_data raw_data{};
    if (bme280_get_sensor_data(BME280_PRESS, &raw_data, &g_sensor) != 0) {
        ESP_LOGE(TAG, "Read failed. Reinitializing...");
        init();
        return std::unexpected(Error::BME280_READ_FAILED);
    }
    data.air_pressure = static_cast<float>(
        raw_data.pressure);  // We don't need double precision
    if (bme280_get_sensor_data(BME280_HUM, &raw_data, &g_sensor) != 0) {
        return std::unexpected(Error::BME280_READ_FAILED);
    }
    data.humidity = static_cast<float>(raw_data.humidity);
    if (bme280_get_sensor_data(BME280_TEMP, &raw_data, &g_sensor) != 0) {
        return std::unexpected(Error::BME280_READ_FAILED);
    }
    data.temperature = static_cast<float>(raw_data.temperature);

    return data;
}

auto readAndProcessData() -> std::expected<Success, Error> {
    auto ret = readData();

    return ret.and_then([](Data const &data) -> std::expected<Success, Error> {
        storage::postBarometerData(data);
        return Success{};
    });
}

}  // namespace bme280
