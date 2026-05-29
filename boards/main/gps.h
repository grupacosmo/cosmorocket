#pragma once

#include <array>

namespace gps {

constexpr inline std::size_t GPS_LINE_SIZE = 64;

using Data = std::array<char, GPS_LINE_SIZE>;

void init();

}  // namespace gps