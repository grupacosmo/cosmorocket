#pragma once

#include <math.h>
#include "helper_3dmath.h"

namespace mpu {

struct Data {
    VectorFloat max;
    VectorFloat avg;
    VectorFloat rot; //in degrees
};

void init();
bool calibrate();
void mpu_task(void *pvParameters);
void mpu_print(void *pvParameters);
void print_data();
Data get_data();

} // namespace mpu
