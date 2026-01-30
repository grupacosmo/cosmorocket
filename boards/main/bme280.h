#pragma once

#include <expected>

#include "common.h"

namespace bme280 {

struct Data {
    float air_pressure;
    float humidity;
    float temperature;
};

auto init() -> Result<Success>;
auto readAndProcessData() -> std::expected<Success, Error>;

}  // namespace bme280