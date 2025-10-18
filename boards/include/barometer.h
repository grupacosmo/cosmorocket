#pragma once

#include "data.h"

namespace barometer {

void init();
void getData(Data::Pressure &pressure, Data::Temperature &temperature);

}  // namespace barometer
