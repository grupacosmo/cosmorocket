#include "mpu.h"
#include "MPU6050.h"

namespace mpu {

MPU6050 mpudev;

unsigned int count = 0;
Data internal{};
float gyro_res = 0, accel_res = 0;
bool init_success = false;

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
}

Data get_data() {
    Data dane{};
    dane.max.x = internal.max.x;
    dane.max.y = internal.max.y;
    dane.max.z = internal.max.z;
    dane.avg.x = internal.avg.x / count,
    dane.avg.y = internal.avg.y / count,
    dane.avg.z = internal.avg.z / count,
    dane.rot.x = internal.rot.x / count,
    dane.rot.y = internal.rot.y / count,
    dane.rot.z = internal.rot.z / count,

    count = 0;
    internal = {};

    return dane;
}

void mpu_task([[maybe_unused]] void *pvParameters) {
    int16_t ax, ay, az, gx, gy, gz;
    float fax, fay, faz, fgx, fgy, fgz;

    if (!init_success) {
        vTaskDelete(nullptr);
    }

    for (;;) {
        mpudev.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        fax = abs(ax * accel_res);
        fay = abs(ay * accel_res);
        faz = abs(az * accel_res);
        fgx = abs(gx * gyro_res);
        fgy = abs(gy * gyro_res);
        fgz = abs(gz * gyro_res);

        if (internal.max.x < fax) {
            internal.max.x = fax;
        }
        if (internal.max.y < fay) {
            internal.max.y = fay;
        }
        if (internal.max.z < faz) {
            internal.max.z = faz;
        }

        internal.avg.x += fax;
        internal.avg.y += fay;
        internal.avg.z += faz;
        internal.rot.x += fgx;
        internal.rot.y += fgy;
        internal.rot.z += fgz;

        count++;

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void print_data() {
    Data data = get_data();
    Serial.printf("[Max g force]\n"
                  "[x, y, z]\n"
                  "%f, %f, %f\n"
                  "[Average g force]\n"
                  "[x, y, z]\n"
                  "%f, %f, %f\n"
                  "[Average rotation]\n"
                  "[x, y, z]\n"
                  "%f, %f, %f\n",
                  data.max.x, data.max.y, data.max.z,
                  data.avg.x, data.avg.y, data.avg.z,
                  data.rot.x, data.rot.y, data.rot.z);
}

} // namespace mpu
