#pragma once

#include "MPU6050.h"

namespace mpu {

struct Data {
    VectorFloat max;
    VectorFloat avg;
    VectorFloat rot;
};

void print_debug();
void init();
bool calibrate();
void mpu_task(void *pvParameters);
void mpu_print(void *pvParameters);
void print_data();
Data get_data();

} // namespace mpu
