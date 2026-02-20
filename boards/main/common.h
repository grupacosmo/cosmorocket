#pragma once

#include <cstdint>
#include <expected>

struct Success {};

enum class Error : uint8_t {
    FAILURE,

    I2C_INIT_FAILED = 0x10,
    I2C_WRITE_FAILED,
    I2C_READ_FAILED,

    BME280_INIT_FAILED = 0x20,
    BME280_READ_FAILED,

    LSM6DSO_INIT_FAILED = 0x30,
    LSM6DSO_READ_FAILED,

    STORAGE_INIT_FAILED = 0x40,
    STORAGE_FILE_CREATION_FAILED,
    STORAGE_FILE_OPENING_FAILED,
    STORAGE_FILE_WRITE_FAILED,
};
