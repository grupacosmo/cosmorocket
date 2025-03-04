#include "mpu.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "display.h"

namespace mpu {

namespace {

MPU6050 mpudev;

unsigned int count = 0;
Data internal{};
float gyro_res = 0, accel_res = 0;
bool init_success = false;

uint16_t packetSize;
uint8_t FIFOBuffer[42];
Quaternion q;
VectorFloat gravity;
float ypr[3];

};

void print_debug()
{
    Adafruit_SSD1306 *disp = display::display_get();

    Data data = get_data();

    disp->printf("Acceleration (max):\n%.4f %.4f %.4f\nRotation:\n%.2f %.2f %.2f\n", data.max.x, data.max.y, data.max.z, data.rot.x, data.rot.y, data.rot.z);
}

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

    mpudev.CalibrateAccel(10);
    mpudev.CalibrateGyro(10);
    mpudev.setDMPEnabled(true);
    packetSize = mpudev.dmpGetFIFOPacketSize();
    accel_res = mpudev.get_acce_resolution() / 2;
    gyro_res = mpudev.get_gyro_resolution() / 2;

    init_success = true;
}

Data get_data() {
    Data dane{};
    dane.max.x = internal.max.x;
    dane.max.y = internal.max.y;
    dane.max.z = internal.max.z;
    dane.avg.x = internal.avg.x / count,
    dane.avg.y = internal.avg.y / count,
    dane.avg.z = internal.avg.z / count,
    dane.rot.x = ypr[0] * 180 / M_PI; //in degrees
    dane.rot.y = ypr[1] * 180 / M_PI;
    dane.rot.z = ypr[2] * 180 / M_PI;

    count = 0;
    internal = {};

    return dane;
}

void mpu_task([[maybe_unused]] void *pvParameters) {
    VectorInt16 iaccel, igyro;
    VectorFloat faccel, fgyro;

    if (!init_success) {
        Serial.println("Mpu is not initialised. Task will now exit.");
        vTaskDelete(nullptr);
    }

    for (;;) {
        if (mpudev.dmpGetCurrentFIFOPacket(FIFOBuffer)) {
            mpudev.dmpGetQuaternion(&q, FIFOBuffer);
            mpudev.dmpGetGravity(&gravity, &q);
            mpudev.dmpGetYawPitchRoll(ypr, &q, &gravity);
            Serial.print("Rotation:\t");
            Serial.print(ypr[0] * 180 / M_PI);
            Serial.print("\t");
            Serial.print(ypr[1] * 180 / M_PI);
            Serial.print("\t");
            Serial.println(ypr[2] * 180 / M_PI);

            mpudev.getMotion6(&iaccel.x, &iaccel.y, &iaccel.z, &igyro.x, &igyro.y, &igyro.z);

            faccel.x = abs(iaccel.x * accel_res);
            faccel.y = abs(iaccel.y * accel_res);
            faccel.z = abs(iaccel.z * accel_res);

            if (internal.max.x < faccel.x) {
                internal.max.x = faccel.x;
            }
            if (internal.max.y < faccel.y) {
                internal.max.y = faccel.y;
            }
            if (internal.max.z < faccel.z) {
                internal.max.z = faccel.z;
            }

            internal.avg.x += faccel.x;
            internal.avg.y += faccel.y;
            internal.avg.z += faccel.z;

            count++;
        }

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
