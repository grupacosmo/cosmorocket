#pragma once

#include <stdint.h>

#include "common.h"

namespace i2c {

void init();
Result read(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size);
Result write(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size);

}  // namespace i2c
