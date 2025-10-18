#pragma once

#include <stdint.h>

namespace i2c {

void init();
bool read(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size);
bool write(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size);

}  // namespace i2c
