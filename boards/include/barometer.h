#pragma once

namespace barometer {

using Pressure = float;
using Temperature = float;

void init();
void getData(Pressure &pressure, Temperature &temperature);

}  // namespace barometer
