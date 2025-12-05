#include "barometer.h"

#include <HardwareSerial.h>
#include <bme280.h>

#include "board_config.h"
#include "i2c.h"
#include "storage.h"

namespace barometer {

// Indicates that an error has occurred during initialization and all subsequent operations will
// fail.
bool g_init_error = false;

// Sensor driver context
struct bme280_dev g_sensor;

// I/O functions passed to bme280 driver
BME280_INTF_RET_TYPE platform_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length,
                                   void *intf_ptr) {
    if (i2c::read(board_config::BME280_ADDR, reg_addr, reg_data, length) != Result::SUCCESS) {
        return -1;
    }
    return 0;
}

BME280_INTF_RET_TYPE platform_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length,
                                    void *intf_ptr) {
    if (i2c::write(board_config::BME280_ADDR, reg_addr, reg_data, length) != Result::SUCCESS) {
        return -1;
    }
    return 0;
}

void bme280_delay_us(uint32_t period, void *intf_ptr) { usleep(period); }

Result init() {
    g_sensor.intf = BME280_I2C_INTF;
    g_sensor.intf_ptr = nullptr;  // Not used
    g_sensor.read = platform_read;
    g_sensor.write = platform_write;
    g_sensor.delay_us = bme280_delay_us;

    int8_t res = bme280_init(&g_sensor);
    if (res != BME280_OK) {
        Serial.println("BME280: sensor not found.");
        g_init_error = true;
        return Result::BAROMETER_INIT_FAILED;
    }

    struct bme280_settings settings;

    // Filter is of no use to us, because we need to know values at the moment, even if they reach
    // extremes and last only a short while
    settings.filter = BME280_FILTER_COEFF_OFF;

    // Only the pressure is oversampled to provide higher accuracy
    settings.osr_h = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_16X;
    settings.osr_t = BME280_OVERSAMPLING_1X;

    // Measure continuously
    settings.standby_time = BME280_STANDBY_TIME_0_5_MS;

    res = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &g_sensor);
    if (res != BME280_OK) {
        Serial.println("BME280: set settings failed.");
        g_init_error = true;
        return Result::BAROMETER_INIT_FAILED;
    }

    res = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &g_sensor);
    if (res != BME280_OK) {
        Serial.println("BME280: set mode failed.");
        g_init_error = true;
        return Result::BAROMETER_INIT_FAILED;
    }

    uint32_t period = 0;
    bme280_cal_meas_delay(&period, &settings);
    Serial.print("BME280: measurement time [ms]: ");
    Serial.println(period / 1000.0f);

    return Result::SUCCESS;
}

static Result readData(float &pressure, float &humidity, float &temperature) {
    pressure = 0.0f;
    humidity = 0.0f;
    temperature = 0.0f;
    if (g_init_error) return Result::BAROMETER_INIT_FAILED;

    struct bme280_data data;
    if (bme280_get_sensor_data(BME280_PRESS, &data, &g_sensor) != 0) {
        Serial.println("BME280: read failed. Reinitializing...");
        init();
        return Result::BAROMETER_READ_FAILED;
    }
    pressure = static_cast<float>(data.pressure);  // We don't need double precision
    if (bme280_get_sensor_data(BME280_HUM, &data, &g_sensor) != 0)
        return Result::BAROMETER_READ_FAILED;
    humidity = static_cast<float>(data.humidity);
    if (bme280_get_sensor_data(BME280_TEMP, &data, &g_sensor) != 0)
        return Result::BAROMETER_READ_FAILED;
    temperature = static_cast<float>(data.temperature);

    return Result::SUCCESS;
}

Result readAndProcessData() {
    Data data;
    Result ret = readData(data.air_pressure, data.humidity, data.temperature);
    if (ret != Result::SUCCESS) {
        return ret;
    }

    storage::postBarometerData(data);

    return Result::SUCCESS;
}

}  // namespace barometer
