#include "mpu.h"
#include "MPU6050_6Axis_MotionApps20.h"

namespace mpu {

namespace {

MPU6050 mpudev;

unsigned int count = 0;
Data internal{};
float gyro_res = 0, accel_res = 0;
uint16_t packetSize;
bool init_success = false;

};

void init() {
    mpudev.initialize();

    if (!mpudev.testConnection()) {
        Serial.println("Connection to mpu failed");
        return;
    }

    if (mpudev.dmpInitialize()) {
        Serial.print("mpu: DMP Initialization failed");
        return;
    }

    mpudev.CalibrateAccel(6);
    mpudev.CalibrateGyro(6);
    mpudev.setDMPEnabled(true);
    packetSize = mpudev.dmpGetFIFOPacketSize();
    accel_res = mpudev.get_acce_resolution() / 2;
    gyro_res = mpudev.get_gyro_resolution() / 2;

    init_success = true;
}

Data get_data() {
    Data dane{};
    dane.acc_max.x = internal.acc_max.x;
    dane.acc_max.y = internal.acc_max.y;
    dane.acc_max.z = internal.acc_max.z;
    dane.acc_avg.x = internal.acc_avg.x / count,
    dane.acc_avg.y = internal.acc_avg.y / count,
    dane.acc_avg.z = internal.acc_avg.z / count,
    dane.rot_avg.x = internal.rot_avg.x / count;
    dane.rot_avg.y = internal.rot_avg.y / count;
    dane.rot_avg.z = internal.rot_avg.z / count;
    dane.rot_avg.w = internal.rot_avg.w / count;

    count = 0;
    internal = {};

    return dane;
}

void mpu_task([[maybe_unused]] void *pvParameters) {
    VectorInt16 iaccel, igyro;

    if (!init_success) {
        Serial.println("Mpu is not initialised. Task will now exit.");
        vTaskDelete(nullptr);
    }

    for (;;) {
        uint8_t FIFOBuffer[42];
        Quaternion q;

        if (mpudev.dmpGetCurrentFIFOPacket(FIFOBuffer)) {
            mpudev.dmpGetQuaternion(&q, FIFOBuffer);

            mpudev.getMotion6(&iaccel.x, &iaccel.y, &iaccel.z, &igyro.x, &igyro.y, &igyro.z);

            if (internal.acc_max.x < iaccel.x) {
                internal.acc_max.x = iaccel.x;
            }
            if (internal.acc_max.y < iaccel.y) {
                internal.acc_max.y = iaccel.y;
            }
            if (internal.acc_max.z < iaccel.z) {
                internal.acc_max.z = iaccel.z;
            }

            internal.acc_avg.x += iaccel.x;
            internal.acc_avg.y += iaccel.y;
            internal.acc_avg.z += iaccel.z;

            internal.rot_avg.x += q.x;
            internal.rot_avg.y += q.y;
            internal.rot_avg.z += q.z;
            internal.rot_avg.w += q.w;

            count++;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void print_data() {
    Data data = get_data();
    Serial.printf("[Max g force]\n"
                  "[x, y, z]\n"
                  "%d, %d, %d\n"
                  "[Average g force]\n"
                  "[x, y, z]\n"
                  "%d, %d, %d\n"
                  "[Average rotation]\n"
                  "[x, y, z, w]\n"
                  "%f, %f, %f, %f\n",
                  data.acc_max.x, data.acc_max.y, data.acc_max.z,
                  data.acc_avg.x, data.acc_avg.y, data.acc_avg.z,
                  data.rot_avg.x, data.rot_avg.y, data.rot_avg.z, data.rot_avg.w);
}

} // namespace mpu
