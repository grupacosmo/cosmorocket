#pragma once

#include <cstddef>
#include <cstdint>

namespace accelerometer {

using Acceleration = int16_t[3];
using AngularRate = int16_t[3];

void init();
void readData();

Acceleration *getAccelerationBuffer();
AngularRate *getAngularRateBuffer();

size_t getAccelelerationCount();
size_t getAngularRateCount();

}  // namespace accelerometer
