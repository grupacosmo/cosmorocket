#pragma once

#include <cstdint>
#include <expected>

#include "common.h"
#include "driver/i2c_types.h"

struct I2CDevice {
    I2CDevice() = default;
    static auto newDevice(uint8_t addr) -> std::expected<I2CDevice, Error>;
    auto write(uint8_t reg, const uint8_t *buffer, uint16_t size)
        -> std::expected<Success, Error>;
    auto read(uint8_t reg, uint8_t *buffer, uint16_t size)
        -> std::expected<Success, Error>;

   private:
    explicit I2CDevice(i2c_master_dev_handle_t dev_handle);
    i2c_master_dev_handle_t dev_handle_;
};
