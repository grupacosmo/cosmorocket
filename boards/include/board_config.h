#pragma once

#include <lsm6dso_reg.h>

#include <cstdint>

namespace board_config {

constexpr inline int I2C_SDA_PIN = 7;
constexpr inline int I2C_SCL_PIN = 6;

constexpr inline uint8_t LSM6DSO_ADDR = LSM6DSO_I2C_ADD_H;  // Alternetively LSM6DSO_I2C_ADD_L

}  // namespace board_config
