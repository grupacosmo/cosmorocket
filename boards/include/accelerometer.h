#pragma once

#include <array>
#include <cstdint>

namespace accelerometer {

using Acceleration = std::array<int16_t, 3>;
using AngularRate = std::array<int16_t, 3>;

constexpr inline size_t ACCEL_BUFFER_SIZE = 256;
constexpr inline size_t ANGULAR_RATE_BUFFER_SIZE = 256;

void init();
void readData();

}  // namespace accelerometer
