#pragma once

#include <cstdint>

enum class Result : uint8_t {
    SUCCESS = 0,
    FAILURE,

    I2C_INIT_FAILED,
    I2C_WRITE_FAILED,
    I2C_READ_FAILED,
};
