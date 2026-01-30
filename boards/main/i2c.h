#pragma once

#include <cstdint>

#include "common.h"

namespace i2c {

auto init() -> Result<Success>;
auto read(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size)
    -> Result<Success>;
auto write(uint8_t addr, uint8_t reg, const uint8_t *buffer, uint16_t size)
    -> Result<Success>;

}  // namespace i2c
