#pragma once

#include <bme280.h>
#include <lsm6dso_reg.h>

#include <cstdint>

namespace board_config {

// If this setting is true, the program will wait for the serial monitor to connect before starting
constexpr inline bool WAIT_FOR_SERIAL = true;

constexpr inline int I2C_SDA_PIN = 7;
constexpr inline int I2C_SCL_PIN = 6;

constexpr inline int GPS_UART_TX_PIN = 33;
constexpr inline int GPS_UART_RX_PIN = 32;

constexpr inline int GPS_BAUDRATE = 115200;

constexpr inline uint8_t LSM6DSO_ADDR = LSM6DSO_I2C_ADD_H;    // Alternetively LSM6DSO_I2C_ADD_L
constexpr inline uint8_t BME280_ADDR = BME280_I2C_ADDR_PRIM;  // Alternatively BME280_I2C_ADDR_SEC

}  // namespace board_config
