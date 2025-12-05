#pragma once

#include <array>
#include <cstdint>

#include "common.h"

namespace accelerometer {

constexpr inline size_t ACCEL_BUFFER_SIZE = 256;
constexpr inline size_t ANGULAR_RATE_BUFFER_SIZE = 256;

Result init();
Result readData();

}  // namespace accelerometer
