#pragma once

#include "accelerometer.h"
#include "barometer.h"
#include "gps.h"

namespace storage {

void init();
void postBarometerData(const barometer::Data &data);
void postAccelerationData(const accelerometer::Acceleration &data);
void postAngularRateData(const accelerometer::AngularRate &data);
void postGpsData(const gps::Data &data);

}  // namespace storage
