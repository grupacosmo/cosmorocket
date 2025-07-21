#include "mpu.h"

#include "MPU6050_6Axis_MotionApps20.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace mpu {

namespace {

MPU6050 mpudev;

unsigned int count = 0;
Data internal{};
uint16_t packetSize;
bool init_success = false;
constexpr uint8_t MPU_CALIBRATION_ITER_CNT = 6;
constexpr uint8_t MAX_PACKETS_PER_CYCLE = 5;

SemaphoreHandle_t mpu_mutex;

};  // namespace

void init() {
  mpu_mutex = xSemaphoreCreateMutex();
  mpudev.initialize();

  if (!mpudev.testConnection()) {
    Serial.println("Connection to mpu failed");
    return;
  }

  if (mpudev.dmpInitialize()) {
    Serial.print("mpu: DMP Initialization failed");
    return;
  }

  mpudev.CalibrateAccel(MPU_CALIBRATION_ITER_CNT);
  mpudev.CalibrateGyro(MPU_CALIBRATION_ITER_CNT);
  mpudev.setDMPEnabled(true);
  packetSize = mpudev.dmpGetFIFOPacketSize();

  init_success = true;
  Serial.println("MPU initialized successfully");
}

Data get_data() {
  Data mpu_data{};

  if (count == 0) return mpu_data;

  mpu_data.acc_max.x = internal.acc_max.x;
  mpu_data.acc_max.y = internal.acc_max.y;
  mpu_data.acc_max.z = internal.acc_max.z;

  mpu_data.gyro_max.x = internal.gyro_max.x,
  mpu_data.gyro_max.y = internal.gyro_max.y,
  mpu_data.gyro_max.z = internal.gyro_max.z,

  mpu_data.acc_avg.x = internal.acc_avg.x / count,
  mpu_data.acc_avg.y = internal.acc_avg.y / count,
  mpu_data.acc_avg.z = internal.acc_avg.z / count,

  mpu_data.gyro_avg.x = internal.gyro_avg.x / count,
  mpu_data.gyro_avg.y = internal.gyro_avg.y / count,
  mpu_data.gyro_avg.z = internal.gyro_avg.z / count,

  mpu_data.rot_avg.x = internal.rot_avg.x / count;
  mpu_data.rot_avg.y = internal.rot_avg.y / count;
  mpu_data.rot_avg.z = internal.rot_avg.z / count;
  mpu_data.rot_avg.w = internal.rot_avg.w / count;

  count = 0;
  internal = {};

  return mpu_data;
}

void process_vectors(VectorInt16 &max, VectorInt16 &avg, VectorInt16 data) {
  if (max.x < data.x) max.x = data.x;
  if (max.y < data.y) max.y = data.y;
  if (max.z < data.z) max.z = data.z;

  avg.x += data.x;
  avg.y += data.y;
  avg.z += data.z;
}

void mpu_task([[maybe_unused]] void *pvParameters) {
  VectorInt16 iaccel, igyro;
  uint8_t FIFOBuffer[42];
  Quaternion q;

  if (!init_success) {
    Serial.println("Mpu is not initialised. Task will now exit.");
    vTaskDelete(nullptr);
  }

  for (;;) {
    if (!mpudev.getDMPEnabled()) {
      Serial.println("Reinitialising MPU");
      mpudev.initialize();
      if (mpudev.testConnection() && mpudev.dmpInitialize() == 0) {
        mpudev.setDMPEnabled(true);
      } else {
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      }
    }
    uint8_t packets_processed = 0;

    while (packets_processed < MAX_PACKETS_PER_CYCLE &&
           mpudev.dmpGetCurrentFIFOPacket(FIFOBuffer)) {
      packets_processed++;

      mpudev.dmpGetQuaternion(&q, FIFOBuffer);
      mpudev.dmpGetAccel(&iaccel, FIFOBuffer);
      mpudev.dmpGetGyro(&igyro, FIFOBuffer);

      if (xSemaphoreTake(mpu_mutex, pdMS_TO_TICKS(10))) {
        process_vectors(internal.acc_max, internal.acc_avg, iaccel);
        process_vectors(internal.gyro_max, internal.gyro_avg, igyro);

        internal.rot_avg.x += q.x;
        internal.rot_avg.y += q.y;
        internal.rot_avg.z += q.z;
        internal.rot_avg.w += q.w;

        count++;
        xSemaphoreGive(mpu_mutex);
      } else {
        Serial.println("Warning: Failed to get MPU mutex in mpu_task()");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
void print_data(Data &data) {
  Serial.printf(
      "[MPU6050]: \n"
      "---------[Max g force] "
      "%d, %d, %d\n"
      "---------[Average g force] "
      "%d, %d, %d\n"
      "---------[Max gyroscope] "
      "%d, %d, %d\n"
      "---------[Average gyroscope] "
      "%d, %d, %d\n"
      "---------[Average rotation] "
      "%f, %f, %f, %f\n",
      data.acc_max.x, data.acc_max.y, data.acc_max.z, data.acc_avg.x,
      data.acc_avg.y, data.acc_avg.z, data.gyro_max.x, data.gyro_max.y,
      data.gyro_max.z, data.gyro_avg.x, data.gyro_avg.y, data.gyro_avg.z,
      data.rot_avg.x, data.rot_avg.y, data.rot_avg.z, data.rot_avg.w);
}

}  // namespace mpu
