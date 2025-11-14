#include "accelerometer.h"

#include <HardwareSerial.h>
#include <RTOS.h>
#include <lsm6dso_reg.h>

#include "board_config.h"
#include "common.h"
#include "i2c.h"

namespace accelerometer {

constexpr inline int SENSOR_BOOT_TIME = 10;  // In milliseconds

union UnionUByteToWord {
    uint8_t u_byte[6];
    int16_t word[3];
};

bool g_init_error = false;
stmdev_ctx_t g_sensor_ctx;
UnionUByteToWord g_data_raw;
std::array<Acceleration, ACCEL_BUFFER_SIZE> g_acceleration_buf;
std::array<AngularRate, ANGULAR_RATE_BUFFER_SIZE> g_angular_rate_buf;
size_t g_acceleration_cnt, g_angular_rate_cnt;

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
    // We bit shift the address by one because the driver includes the read/write bit, which is not
    // needed, because the I2C implementation already adds it automatically
    if (i2c::write(board_config::LSM6DSO_ADDR >> 1, reg, bufp, len) != 0) return 1;
    return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    // We bit shift the address by one because the driver includes the read/write bit, which is not
    // needed, because the I2C implementation already adds it automatically
    if (i2c::read(board_config::LSM6DSO_ADDR >> 1, reg, bufp, len) != 0) return 1;
    return 0;
}

static void platform_delay(uint32_t ms) { usleep(ms * 1000); }

void init() {
    g_sensor_ctx.write_reg = platform_write;
    g_sensor_ctx.read_reg = platform_read;
    g_sensor_ctx.mdelay = platform_delay;
    g_sensor_ctx.handle = nullptr;     // Not used
    g_sensor_ctx.priv_data = nullptr;  // Not used

    // Wait for the sensor to boot
    vTaskDelay(SENSOR_BOOT_TIME / portTICK_PERIOD_MS);
    uint8_t device_id;
    if (lsm6dso_device_id_get(&g_sensor_ctx, &device_id) != 0) {
        Serial.println("LSM6DSO: sensor not found.");
        g_init_error = true;
        return;
    }
    if (device_id != LSM6DSO_ID) {
        Serial.println("LSM6DSO: invalid device id.");
        g_init_error = true;
        return;
    }
    lsm6dso_reset_set(&g_sensor_ctx, PROPERTY_ENABLE);
    uint8_t reset;
    do {
        vTaskDelay(1 / portTICK_PERIOD_MS);
        if (lsm6dso_reset_get(&g_sensor_ctx, &reset) != 0) {  // Stop if an error occurs
            g_init_error = true;
            return;
        }
    } while (reset);

    lsm6dso_i3c_disable_set(&g_sensor_ctx, LSM6DSO_I3C_DISABLE);
    lsm6dso_block_data_update_set(&g_sensor_ctx, PROPERTY_ENABLE);

    lsm6dso_xl_full_scale_set(&g_sensor_ctx, LSM6DSO_16g);
    lsm6dso_gy_full_scale_set(&g_sensor_ctx, LSM6DSO_2000dps);

    lsm6dso_fifo_xl_batch_set(&g_sensor_ctx, LSM6DSO_XL_BATCHED_AT_833Hz);
    lsm6dso_fifo_gy_batch_set(&g_sensor_ctx, LSM6DSO_GY_BATCHED_AT_833Hz);

    lsm6dso_fifo_mode_set(&g_sensor_ctx, LSM6DSO_STREAM_MODE);

    lsm6dso_xl_data_rate_set(&g_sensor_ctx, LSM6DSO_XL_ODR_833Hz);
    lsm6dso_gy_data_rate_set(&g_sensor_ctx, LSM6DSO_GY_ODR_833Hz);
}

template <typename T>
Result readFifoEntry(T *buffer, size_t buffer_size, size_t &index) {
    if (lsm6dso_fifo_out_raw_get(&g_sensor_ctx, g_data_raw.u_byte) != 0) return FAILURE;
    if (index < buffer_size) {  // Only if there is space left in the buffer
        auto &acceleration = buffer[index++];
        acceleration[0] = g_data_raw.word[0];
        acceleration[1] = g_data_raw.word[1];
        acceleration[2] = g_data_raw.word[2];
    }
    return SUCCESS;
}

void readData() {
    lsm6dso_fifo_tag_t tag;
    uint16_t data_count;

    // Reset buffer size
    g_acceleration_cnt = 0;
    g_angular_rate_cnt = 0;

    if (g_init_error) return;

    // Check number of samples stored in FIFO
    if (lsm6dso_fifo_data_level_get(&g_sensor_ctx, &data_count) != 0) {
        // Failed to read data from the sensor
        Serial.println("LSM6DSO: read failed. Reinitialising...");
        init();
        return;
    }

    // Serial.printf("LSM6DSO: reading %d measurements from FIFO\n", data_count);

    while (data_count--) {
        lsm6dso_fifo_sensor_tag_get(&g_sensor_ctx, &tag);
        switch (tag) {
            case LSM6DSO_XL_NC_TAG: {
                auto res =
                    readFifoEntry(g_acceleration_buf.data(), ACCEL_BUFFER_SIZE, g_acceleration_cnt);
                if (res != 0) return;
                break;
            }
            case LSM6DSO_GYRO_NC_TAG: {
                auto res = readFifoEntry(g_angular_rate_buf.data(), ANGULAR_RATE_BUFFER_SIZE,
                                         g_angular_rate_cnt);
                if (res != 0) return;
                break;
            }
            default: {
                // Even if we don't use the data type, it still needs to be read
                Serial.println("LSM6DSO: excessive data found");
                auto res = lsm6dso_fifo_out_raw_get(&g_sensor_ctx, g_data_raw.u_byte);
                if (res != 0) return;
                break;
            }
        }
    }
}

std::array<Acceleration, ACCEL_BUFFER_SIZE> &getAccelerationBuffer() { return g_acceleration_buf; }

std::array<AngularRate, ANGULAR_RATE_BUFFER_SIZE> &getAngularRateBuffer() {
    return g_angular_rate_buf;
}

size_t getAccelelerationCount() { return g_acceleration_cnt; }

size_t getAngularRateCount() { return g_angular_rate_cnt; }

}  // namespace accelerometer
