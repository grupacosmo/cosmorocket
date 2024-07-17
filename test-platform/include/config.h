#pragma once
#include <cstdint>
namespace Config {
constexpr uint8_t sda = 5;
constexpr uint8_t scl = 17;
constexpr unsigned long baud_rate = 115200;

constexpr char const *ssid = "SSID";
constexpr char const *password = "PASSWORD";

/// INA226 I2C address, for the specific board we're using it's 0x40
constexpr uint8_t ina_addr = 0x40;

/// Theoretical max output of the PT5423 pressure sensor, in amperes
constexpr float max_current_amperes = 0.080;

/// Shunt resistor on the INA226 board, in Ohms
constexpr float shunt_resistor = 1;
/// value used in converting current to pressure
/// Number corresponding to the number of averages INA226 makes
/// it should be a whole number between 0 and 7 inclusive
/// 7 corresponds to 1024 averages, for details consult table 6
/// in INA226 datasheet
constexpr uint8_t avg = 7;

} // namespace Config
