#pragma once

#include <cstddef>

#include "common.h"

namespace lsm6dso {

constexpr inline size_t ACCEL_BUFFER_SIZE = 256;
constexpr inline size_t ANGULAR_RATE_BUFFER_SIZE = 256;

auto init() -> Result<Success>;
auto readAndProcessData() -> Result<Success>;

}  // namespace lsm6dso