#pragma once
#include "FS.h"
#include "SD.h"
#include "SPI.h"

namespace sd {
static constexpr std::uint8_t DEFAULT_DATA_PIN = 32U;

void init();
void write(const char *data);
void write(const String &data);
void read(const char *filename);
void read(const String &filename);
}; // namespace sd