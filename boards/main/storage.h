#pragma once

#include <array>

#include "bme280.h"
#include "common.h"

namespace storage {

auto init() -> std::expected<Success, Error>;
auto start() -> std::expected<Success, Error>;
void postBarometerData(const bme280::Data &data);
void postAccelerationData(const std::array<int16_t, 3> &data);
void postAngularRateData(const std::array<int16_t, 3> &data);
// void postGpsData(const gps::Data &data);

}  // namespace storage
