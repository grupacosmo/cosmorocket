#pragma once

#include "data.h"

namespace barometer {

void init();
void measure();
Data::Temperature getTemperature();
Data::Pressure getPressure();

}  // namespace barometer
