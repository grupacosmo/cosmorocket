#include "lsm6dso.h"

#include <freertos/FreeRTOS.h>
#include <lsm6dso-pid/lsm6dso_reg.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <expected>

#include "common.h"
#include "esp_log.h"
#include "i2c.h"
#include "sdkconfig.h"
#include "storage.h"

namespace lsm6dso {

constexpr inline const char *TAG = "LSM6DSO";

constexpr inline int SENSOR_BOOT_TIME = 10;  // In milliseconds
constexpr inline size_t RAW_DATA_BUFFER_SIZE = 6;

// Indicates that an error has occurred during initialization and all subsequent
// operations will fail.
bool g_init_error = false;

// Sensor driver object
stmdev_ctx_t g_sensor_ctx;

// I/O functions passed to the lsm6dso driver:
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len) {
    // We bit shift the address by one because the driver includes the
    // read/write bit, which is not needed, because the I2C implementation
    // already adds it automatically
    return i2c::write(CONFIG_LSM6DSO_I2C_ADDR, reg, bufp, len)
        .transform([](Success) -> int32_t { return 0; })
        .value_or(1);
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len) {
    // We bit shift the address by one because the driver includes the
    // read/write bit, which is not needed, because the I2C implementation
    // already adds it automatically
    return i2c::read(CONFIG_LSM6DSO_I2C_ADDR, reg, bufp, len)
        .transform([](Success) -> int32_t { return 0; })
        .value_or(1);
}

static void platform_delay(uint32_t ms) { usleep(ms * 1000); }

auto init() -> Result<Success> {
    g_sensor_ctx.write_reg = platform_write;
    g_sensor_ctx.read_reg = platform_read;
    g_sensor_ctx.mdelay = platform_delay;
    g_sensor_ctx.handle = nullptr;     // Not used
    g_sensor_ctx.priv_data = nullptr;  // Not used

    // Wait for the sensor to boot
    vTaskDelay(SENSOR_BOOT_TIME / portTICK_PERIOD_MS);
    uint8_t device_id = 0;
    if (lsm6dso_device_id_get(&g_sensor_ctx, &device_id) != 0) {
        ESP_LOGE(TAG, "sensor not found.");
        g_init_error = true;
        return std::unexpected(Error::LSM6DSO_INIT_FAILED);
    }
    if (device_id != LSM6DSO_ID) {
        ESP_LOGE(TAG, "invalid device id.");
        g_init_error = true;
        return std::unexpected(Error::LSM6DSO_INIT_FAILED);
    }
    lsm6dso_reset_set(&g_sensor_ctx, PROPERTY_ENABLE);
    uint8_t reset = 0;
    do {
        vTaskDelay(1 / portTICK_PERIOD_MS);
        if (lsm6dso_reset_get(&g_sensor_ctx, &reset) !=
            0) {  // Stop if an error occurs
            g_init_error = true;
            return std::unexpected(Error::LSM6DSO_INIT_FAILED);
        }
    } while (reset);

    lsm6dso_i3c_disable_set(&g_sensor_ctx, LSM6DSO_I3C_DISABLE);
    lsm6dso_block_data_update_set(&g_sensor_ctx, PROPERTY_ENABLE);

    lsm6dso_xl_full_scale_set(&g_sensor_ctx, LSM6DSO_16g);
    lsm6dso_gy_full_scale_set(&g_sensor_ctx, LSM6DSO_2000dps);

    // Setup internal FIFO. The sensor will collect measurements in its internal
    // memory and store it until the main task reads it (multiple measurements
    // are read at once).
    lsm6dso_fifo_xl_batch_set(&g_sensor_ctx, LSM6DSO_XL_BATCHED_AT_417Hz);
    lsm6dso_fifo_gy_batch_set(&g_sensor_ctx, LSM6DSO_GY_BATCHED_AT_417Hz);

    lsm6dso_fifo_mode_set(&g_sensor_ctx, LSM6DSO_STREAM_MODE);

    lsm6dso_xl_data_rate_set(&g_sensor_ctx, LSM6DSO_XL_ODR_417Hz);
    lsm6dso_gy_data_rate_set(&g_sensor_ctx, LSM6DSO_GY_ODR_417Hz);

    return Success{};
}

static std::array<int16_t, 3> convertToSignedShort(
    const std::array<uint8_t, RAW_DATA_BUFFER_SIZE> &raw_data_buffer) {
    std::array<int16_t, 3> tmp{};
    std::memcpy(tmp.data(), raw_data_buffer.data(),
                RAW_DATA_BUFFER_SIZE * sizeof(uint8_t));
    return tmp;
}

auto readAndProcessData() -> Result<Success> {
    lsm6dso_fifo_tag_t tag{};
    uint16_t data_count = 0;

    if (g_init_error) {
        return std::unexpected(Error::LSM6DSO_INIT_FAILED);
    };

    // Check number of samples stored in FIFO
    if (lsm6dso_fifo_data_level_get(&g_sensor_ctx, &data_count) != 0) {
        // Failed to read data from the sensor
        ESP_LOGE(TAG, "read failed. Reinitializing...");
        init();
        return std::unexpected(Error::LSM6DSO_READ_FAILED);
    }

    // Serial.printf("LSM6DSO: reading %d measurements from FIFO\n",
    // data_count);

    std::array<uint8_t, RAW_DATA_BUFFER_SIZE> raw_data_buffer{};

    while (data_count--) {
        lsm6dso_fifo_sensor_tag_get(&g_sensor_ctx, &tag);
        switch (tag) {
            case LSM6DSO_XL_NC_TAG: {
                if (lsm6dso_fifo_out_raw_get(&g_sensor_ctx,
                                             raw_data_buffer.data()) != 0) {
                    return std::unexpected(Error::LSM6DSO_READ_FAILED);
                }
                storage::postAccelerationData(
                    convertToSignedShort(raw_data_buffer));
                break;
            }
            case LSM6DSO_GYRO_NC_TAG: {
                if (lsm6dso_fifo_out_raw_get(&g_sensor_ctx,
                                             raw_data_buffer.data()) != 0) {
                    return std::unexpected(Error::LSM6DSO_READ_FAILED);
                }
                storage::postAngularRateData(
                    convertToSignedShort(raw_data_buffer));
                break;
            }
            default: {
                // Even if we don't use the data type, it still needs to be read
                // to free the internal FIFO
                ESP_LOGE(TAG, "excessive data found");
                auto res = lsm6dso_fifo_out_raw_get(&g_sensor_ctx,
                                                    raw_data_buffer.data());
                if (res != 0) {
                    return std::unexpected(Error::LSM6DSO_READ_FAILED);
                }
                break;
            }
        }
    }

    return Success{};
}

}  // namespace lsm6dso