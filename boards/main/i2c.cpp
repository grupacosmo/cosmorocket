#include "i2c.h"

#include <cstring>

#include "driver/i2c.h"
#include "esp_log.h"

namespace i2c {

constexpr inline const char *TAG = "I2C";

// The project uses only one I2C bus
constexpr inline i2c_port_t BUS_NUMBER = I2C_NUM_0;

constexpr inline int FREQUENCY = 400000;

constexpr inline uint32_t TIMEOUT = 50;

Result init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_I2C_SDA_GPIO,
        .scl_io_num = CONFIG_I2C_SCL_GPIO,
        .sda_pullup_en = true,
        .scl_pullup_en = true,
        .master =
            {
                .clk_speed = FREQUENCY,
            },
        .clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL,
    };

    if (i2c_param_config(BUS_NUMBER, &conf) != ESP_OK) {
        ESP_LOGE(TAG, "Config error");
        return Result::I2C_INIT_FAILED;
    }

    if (i2c_driver_install(BUS_NUMBER, conf.mode, 0, 0, 0) != ESP_OK) {
        ESP_LOGE(TAG, "Init error");
        return Result::I2C_INIT_FAILED;
    }

    return Result::SUCCESS;
}

Result read(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size) {
    if (i2c_master_write_read_device(BUS_NUMBER, addr, &reg, 1, buffer, size,
                                     TIMEOUT / portTICK_PERIOD_MS) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read");
        return Result::I2C_READ_FAILED;
    }

    return Result::SUCCESS;
}

static Result performWriteTransaction(uint8_t addr, uint8_t reg,
                                      const uint8_t *buffer, uint16_t size,
                                      i2c_cmd_handle_t command) {
    if (i2c_master_start(command) != ESP_OK) {
        return Result::I2C_WRITE_FAILED;
    }
    if (i2c_master_write_byte(command, addr << 1U, true) != ESP_OK) {
        return Result::I2C_WRITE_FAILED;
    }
    if (i2c_master_write_byte(command, reg, true) != ESP_OK) {
        return Result::I2C_WRITE_FAILED;
    }
    if (i2c_master_write(command, buffer, size, true) != ESP_OK) {
        return Result::I2C_WRITE_FAILED;
    }
    if (i2c_master_stop(command) != ESP_OK) {
        return Result::I2C_WRITE_FAILED;
    }
    if (i2c_master_cmd_begin(BUS_NUMBER, command, TIMEOUT) != ESP_OK) {
        return Result::I2C_WRITE_FAILED;
    }
    return Result::SUCCESS;
}

Result write(uint8_t addr, uint8_t reg, const uint8_t *buffer, uint16_t size) {
    uint8_t command_buffer[I2C_LINK_RECOMMENDED_SIZE(2)] = {0};
    i2c_cmd_handle_t command = i2c_cmd_link_create_static(
        command_buffer, I2C_LINK_RECOMMENDED_SIZE(2));

    Result res = performWriteTransaction(addr, reg, buffer, size, command);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "Failed to write");
        i2c_cmd_link_delete_static(command);

        return res;
    }

    i2c_cmd_link_delete_static(command);

    return Result::SUCCESS;
}

}  // namespace i2c
