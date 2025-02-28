#include "mpu.h"

#include "MPU6050.h"

namespace mpu {

namespace {

MPU6050 mpudev;

unsigned int count = 0;
float gyro_res = 0, accel_res = 0;
bool init_success = false;

};  // namespace

void init() {
  mpudev.initialize();

  // Calibration on start?
  // mpudev.CalibrateAccel(10);
  // mpudev.CalibrateGyro(10);

  // Alternatively, we can set offsets obtained by running IMU_Zero
  // example from the library.
  // mpudev.setXAccelOffset(0);
  // mpudev.setYAccelOffset(0);
  // mpudev.setZAccelOffset(0);
  // mpudev.setXGyroOffset(0);
  // mpudev.setYGyroOffset(0);
  // mpudev.setZGyroOffset(0);

  if (!mpudev.testConnection()) {
    Serial.println("Connection to mpu failed");
    return;
  }

  init_success = true;
  accel_res = mpudev.get_acce_resolution() / 2;
  gyro_res = mpudev.get_gyro_resolution() / 2;
  Serial.println("MPU6050 initialised");
}

void mpu_task([[maybe_unused]] void* pvParameters) {
  VectorInt16 iaccel, igyro;
  VectorFloat faccel, fgyro;
  Data* data_ptr = (Data*)pvParameters;

  if (!init_success) {
    Serial.println("Mpu is not initialised. Task will now exit.");
    vTaskDelete(nullptr);
  }
  // Serial.println("MPU6050 task started");
  for (;;) {
    mpudev.getMotion6(&iaccel.x, &iaccel.y, &iaccel.z, &igyro.x, &igyro.y,
                      &igyro.z);

    faccel.x = abs(iaccel.x * accel_res);
    faccel.y = abs(iaccel.y * accel_res);
    faccel.z = abs(iaccel.z * accel_res);
    fgyro.x = abs(igyro.x * gyro_res);
    fgyro.y = abs(igyro.y * gyro_res);
    fgyro.z = abs(igyro.z * gyro_res);

    if (data_ptr->max.x < faccel.x) {
      data_ptr->max.x = faccel.x;
    }
    if (data_ptr->max.y < faccel.y) {
      data_ptr->max.y = faccel.y;
    }
    if (data_ptr->max.z < faccel.z) {
      data_ptr->max.z = faccel.z;
    }

    data_ptr->avg.x =
        data_ptr->avg.x + (faccel.x - data_ptr->avg.x) / (count + 1);
    data_ptr->avg.y =
        data_ptr->avg.y + (faccel.y - data_ptr->avg.y) / (count + 1);
    data_ptr->avg.z =
        data_ptr->avg.z + (faccel.z - data_ptr->avg.z) / (count + 1);
    data_ptr->rot.x += fgyro.x;
    data_ptr->rot.y += fgyro.y;
    data_ptr->rot.z += fgyro.z;

    count++;
#ifdef DEBUG
    // print_data(*data_ptr);
#endif
  }
}

void print_data(const Data& data) {
  Serial.printf(
      "[Max g force] [Average g force] [Average rotation]\n"
      "%f, %f, %f,    %f, %f, %f,       %f, %f, %f\n",
      data.max.x, data.max.y, data.max.z, data.avg.x, data.avg.y, data.avg.z,
      data.rot.x, data.rot.y, data.rot.z);
}

}  // namespace mpu
