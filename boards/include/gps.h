#pragma once

#include <array>

#include "nmea_decoder.h"

namespace gps {

using Data = std::array<char, nmea_decoder::MAX_OUTPUT_SIZE>;

void init();

}  // namespace gps
