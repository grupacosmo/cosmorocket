#pragma once

#include <cstdint>

#include "common.h"

namespace i2c {

auto init() -> std::expected<Success, Error>;
auto read(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size)
    -> std::expected<Success, Error>;
auto write(uint8_t addr, uint8_t reg, const uint8_t *buffer, uint16_t size)
    -> std::expected<Success, Error>;

}  // namespace i2c
