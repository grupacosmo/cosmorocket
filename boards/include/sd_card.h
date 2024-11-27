#pragma once
#include "FS.h"
#include "SD.h"
#include "SPI.h"

namespace sd {

void init();
void write(const char *data);
void write(const String &data);
void read(const char *filename);
void read(const String &filename);

};  // namespace sd