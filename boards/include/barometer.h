#pragma once

#include "common.h"

namespace barometer {

struct Data {
    float air_pressure;
    float humidity;
    float temperature;
};

Result init();
Result readAndProcessData();

}  // namespace barometer
