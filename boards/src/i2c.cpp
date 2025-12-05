#include "i2c.h"

#include <HardwareSerial.h>
#include <driver/i2c.h>

#include <cstring>

#include "board_config.h"

namespace i2c {

// The project uses only one I2C bus
constexpr inline uint8_t BUS_NUMBER = I2C_NUM_0;

constexpr inline int FREQUENCY = 400000;

constexpr inline int BUS_TIMEOUT = 0xFFFFF;

constexpr inline uint32_t TIMEOUT = 50;

constexpr inline size_t MAX_SUPPORTED_TRANSFER_SIZE = 255;

Result init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = board_config::I2C_SDA_PIN,
        .scl_io_num = board_config::I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master =
            {
                .clk_speed = FREQUENCY,
            },
        .clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL,
    };

    if (i2c_param_config(BUS_NUMBER, &conf) != ESP_OK) {
        Serial.println("I2C: Config error");
        return Result::I2C_INIT_FAILED;
    }

    if (i2c_driver_install(BUS_NUMBER, conf.mode, 0, 0, 0) != ESP_OK) {
        Serial.println("I2C: Init error");
        return Result::I2C_INIT_FAILED;
    }

    i2c_set_timeout(BUS_NUMBER, BUS_TIMEOUT);
    return Result::SUCCESS;
}

Result read(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size) {
    if (size > MAX_SUPPORTED_TRANSFER_SIZE) {
        Serial.println("I2C: Error: unsupported read size");
        return Result::I2C_READ_FAILED;
    }

    if (i2c_master_write_read_device(BUS_NUMBER, addr, &reg, 1, buffer, size,
                                     TIMEOUT / portTICK_RATE_MS) != ESP_OK) {
        Serial.println("I2C: Failed to read");
        return Result::I2C_READ_FAILED;
    }

    return Result::SUCCESS;
}

static Result performWriteTransaction(uint8_t addr, uint8_t reg, const uint8_t *buffer,
                                      uint16_t size, i2c_cmd_handle_t command) {
    if (i2c_master_start(command) != ESP_OK) {
        return Result::I2C_WRITE_FAILED;
    }
    if (i2c_master_write_byte(command, addr << 1, true) != ESP_OK) {
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
    if (size > MAX_SUPPORTED_TRANSFER_SIZE) {
        Serial.println("I2C: Error: unsupported write size");
        return Result::I2C_WRITE_FAILED;
    }

    uint8_t command_buffer[I2C_LINK_RECOMMENDED_SIZE(2)] = {0};
    i2c_cmd_handle_t command =
        i2c_cmd_link_create_static(command_buffer, I2C_LINK_RECOMMENDED_SIZE(2));

    Result res = performWriteTransaction(addr, reg, buffer, size, command);
    if (res != Result::SUCCESS) {
        Serial.println("I2C: Failed to write");
        i2c_cmd_link_delete_static(command);

        return res;
    }

    i2c_cmd_link_delete_static(command);

    return Result::SUCCESS;
}

}  // namespace i2c
