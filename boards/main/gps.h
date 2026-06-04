#pragma once

#include <cstdint>
#include <variant>

#include "minmea/minmea.h"

namespace gps {

constexpr inline std::size_t GPS_LINE_SIZE = 40;

struct Data {
    int64_t timestamp;
    std::variant<minmea_sentence_gga, minmea_sentence_rmc, minmea_sentence_gll,
                 minmea_sentence_vtg>
        parsed_data;
};

void init();

}  // namespace gps