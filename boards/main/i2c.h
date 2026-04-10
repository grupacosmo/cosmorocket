#pragma once

#include <cstdint>

#include "common.h"
#include "driver/i2c_types.h"

namespace i2c {

auto init() -> std::expected<Success, Error>;
auto getHandle() -> i2c_master_bus_handle_t;

}  // namespace i2c
