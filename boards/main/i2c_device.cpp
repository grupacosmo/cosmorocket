#include "i2c_device.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <expected>

#include "common.h"
#include "driver/i2c_master.h"
#include "i2c.h"

std::array<uint8_t, 256> g_i2c_buffer;

auto I2CDevice::newDevice(uint8_t addr) -> std::expected<I2CDevice, Error> {
    i2c_device_config_t dev_cfg = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                   .device_address = addr,
                                   .scl_speed_hz = 100000,  // Frequency too low
                                   .scl_wait_us = 0};

    i2c_master_dev_handle_t dev_handle;
    if (i2c_master_bus_add_device(i2c::getHandle(), &dev_cfg, &dev_handle) !=
        ESP_OK) {
        return std::unexpected(Error::I2C_DEVICE_REGISTER_FAILED);
    }

    return I2CDevice{dev_handle};
}

auto I2CDevice::write(uint8_t reg, const uint8_t *buffer, uint16_t size)
    -> std::expected<Success, Error> {
    assert(size < 256 - 1);
    g_i2c_buffer[0] = reg;
    for (size_t i = 0; i < size; i++) {
        g_i2c_buffer[1 + i] = buffer[i];
    }
    // mainSemaphoreTake();
    if (i2c_master_transmit(dev_handle_, g_i2c_buffer.data(), 1 + size, -1) !=
        ESP_OK) {
        return std::unexpected(Error::I2C_DEVICE_WRITE_FAILED);
    }
    // mainSemaphoreGive();

    return Success{};
}

auto I2CDevice::read(uint8_t reg, uint8_t *buffer, uint16_t size)
    -> std::expected<Success, Error> {
    // mainSemaphoreTake();
    if (i2c_master_transmit_receive(dev_handle_, &reg, 1, buffer, size, -1) !=
        ESP_OK) {
        return std::unexpected(Error::I2C_DEVICE_READ_FAILED);
    }
    // mainSemaphoreGive();

    return Success{};
}

I2CDevice::I2CDevice(i2c_master_dev_handle_t dev_handle)
    : dev_handle_(dev_handle) {}
