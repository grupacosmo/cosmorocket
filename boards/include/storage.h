#pragma once

#include "accelerometer.h"
#include "barometer.h"
#include "gps.h"

namespace storage {

void init();
void postBarometerData(const barometer::Data &data);
void postAccelerationData(const std::array<int16_t, 3> &data);
void postAngularRateData(const std::array<int16_t, 3> &data);
void postGpsData(const gps::Data &data);

}  // namespace storage
