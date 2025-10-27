#pragma once

#include "common.h"

namespace barometer {

using Pressure = float;
using Humidity = float;
using Temperature = float;

void init();
Result getData(Pressure &pressure, Humidity &humidity, Temperature &temperature);

}  // namespace barometer
