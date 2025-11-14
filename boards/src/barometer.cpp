#include "barometer.h"

#include <HardwareSerial.h>
#include <bme280.h>

#include "board_config.h"
#include "i2c.h"
#include "storage.h"

namespace barometer {

bool g_init_error = false;
struct bme280_dev g_sensor;
struct bme280_data g_data;

BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length,
                                     void *intf_ptr) {
    if (i2c::read(board_config::BME280_ADDR, reg_addr, reg_data, length) != 0) return -1;
    return 0;
}

BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length,
                                      void *intf_ptr) {
    if (i2c::write(board_config::BME280_ADDR, reg_addr, reg_data, length) != 0) return -1;
    return 0;
}

void bme280_delay_us(uint32_t period, void *intf_ptr) { usleep(period); }

void init() {
    g_sensor.intf = BME280_I2C_INTF;
    g_sensor.intf_ptr = nullptr;  // Not used
    g_sensor.read = bme280_i2c_read;
    g_sensor.write = bme280_i2c_write;
    g_sensor.delay_us = bme280_delay_us;

    int8_t rslt = bme280_init(&g_sensor);
    if (rslt != BME280_OK) {
        Serial.println("BME280: sensor not found.");
        g_init_error = true;
        return;
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

    rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &g_sensor);
    if (rslt != BME280_OK) {
        Serial.println("BME280: set settings failed.");
        g_init_error = true;
        return;
    }

    rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &g_sensor);
    if (rslt != BME280_OK) {
        Serial.println("BME280: set mode failed.");
        g_init_error = true;
        return;
    }

    uint32_t period = 0;
    bme280_cal_meas_delay(&period, &settings);
    Serial.print("BME280: measurement time [ms]: ");
    Serial.println(period / 1000.0f);
}

static Result readData(Pressure &pressure, Humidity &humidity, Temperature &temperature) {
    pressure = 0.0f;
    humidity = 0.0f;
    temperature = 0.0f;
    if (g_init_error) return FAILURE;

    if (bme280_get_sensor_data(BME280_PRESS, &g_data, &g_sensor) != 0) {
        Serial.println("BME280: read failed. Reinitialising...");
        init();
        return FAILURE;
    }
    pressure = static_cast<float>(g_data.pressure);  // We don't need double precision
    if (bme280_get_sensor_data(BME280_HUM, &g_data, &g_sensor) != 0) return FAILURE;
    humidity = static_cast<float>(g_data.humidity);
    if (bme280_get_sensor_data(BME280_TEMP, &g_data, &g_sensor) != 0) return FAILURE;
    temperature = static_cast<float>(g_data.temperature);

    return SUCCESS;
}

void readAndProcessData() {
    Data data;
    readData(data.air_pressure, data.humidity, data.temperature);

    storage::postBarometerData(data);
}

}  // namespace barometer
