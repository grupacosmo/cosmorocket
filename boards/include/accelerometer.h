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

std::array<Acceleration, ACCEL_BUFFER_SIZE> &getAccelerationBuffer();
std::array<AngularRate, ANGULAR_RATE_BUFFER_SIZE> &getAngularRateBuffer();

size_t getAccelelerationCount();
size_t getAngularRateCount();

}  // namespace accelerometer
