#pragma once

#include <math.h>

#include "helper_3dmath.h"

namespace mpu {

struct Data {
  VectorInt16 acc_max;  // receiver must multiply acc by 0.0001220703125
  VectorInt16 acc_avg;

  VectorInt16 gyro_max;  // receiver must multiply gyroscope by 0.0152587890625
  VectorInt16 gyro_avg;

  Quaternion rot_avg;
};

void init();
bool calibrate();
void mpu_task(void *pvParameters);
void print_data(Data &mpu_data);
Data get_data();

}  // namespace mpu
