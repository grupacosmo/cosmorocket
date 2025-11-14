#pragma once

#include "common.h"

namespace barometer {

using Pressure = float;
using Humidity = float;
using Temperature = float;

struct Data {
    Pressure air_pressure;
    Humidity humidity;
    Temperature temperature;
};

void init();
void readAndProcessData();

}  // namespace barometer
