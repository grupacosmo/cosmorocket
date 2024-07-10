#pragma once
#include "FS.h"
#include "SD.h"
#include "SPI.h"

namespace sd {
static constexpr std::uint8_t DEFAULT_DATA_PIN = 33U;

void init();
void write(const char *data, const char *filename);
void write(const String &data, const String &filename );
void read(const char *filename);
void read(const String &filename);
}; // namespace sd