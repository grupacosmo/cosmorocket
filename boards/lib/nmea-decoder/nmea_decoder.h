#pragma once

#include <array>

namespace nmea_decoder {

static constexpr inline int MAX_SENTENCE_SIZE = 196;
static constexpr inline int MAX_OUTPUT_SIZE = 50;

enum Result { SUCCESS = 0, WRONG_HEADER, WRONG_DATA_FORMAT, BUFFER_OVERFLOW };

Result decode(std::array<char, MAX_OUTPUT_SIZE> &out,
              const std::array<char, MAX_SENTENCE_SIZE> &nmea_sentence);

}  // namespace nmea_decoder
