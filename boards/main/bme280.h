#pragma once

#include <cstdint>
#include <expected>

#include "common.h"

namespace bme280 {

struct __attribute__((packed)) Data {
    int64_t timestamp;
    float air_pressure;
    float humidity;
    float temperature;
};

auto init() -> std::expected<Success, Error>;
auto readAndProcessData() -> std::expected<Success, Error>;

}  // namespace bme280