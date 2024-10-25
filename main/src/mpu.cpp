#include "mpu.h"
#include "MPU6050.h"

namespace mpu {

MPU6050 mpudev;

static SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
static unsigned int count = 0;
static Data internal{};
static float gyro_res = 0, accel_res = 0;
static bool init_success = false;

/**
 * @brief Mpu initialization. Return gracefully on initialisation error.
 */
void init() {
    // Parameters change the resolution of the sensor.
    mpudev.initialize(/*MPU6050_ACCEL_FS_2, MPU6050_GYRO_FS_250*/);

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
        // die
        Serial.println("Connection to mpu failed");
        return;
    }

    if (mutex == nullptr) {
        // die
        Serial.println("Couldn't allocate resources for mutex!");
        return;
    }

    init_success = true;
    accel_res = mpudev.get_acce_resolution() / 2;
    gyro_res = mpudev.get_gyro_resolution() / 2;
}

/**
 * @brief Consume and return collected data.
 *        ! CAN BLOCK INDEFINITELY (if INCLUDE_vTaskSuspend == 1) !
 * @return Max g force, Average g force, Average rotation.
 */
Data get_mpu() {
    xSemaphoreTake(mutex, portMAX_DELAY);

    Data dane{
        .max_x = internal.max_x,
        .max_y = internal.max_y,
        .max_z = internal.max_z,
        .avg_x = internal.avg_x / count, // Instead of counting could implement
        .avg_y = internal.avg_y / count, // moving average, but dunno if it's
        .avg_z = internal.avg_z / count, // superior and/or viable alternative.
        .rot_x = internal.rot_x / count,
        .rot_y = internal.rot_y / count,
        .rot_z = internal.rot_z / count};

    count = 0;
    internal = {};

    xSemaphoreGive(mutex);

    return dane;
}

/**
 * @brief Main MPU task that collects data from the sensor.
 *        Die gracefully if sensor not initialised.
 *        ! CAN BLOCK INDEFINITELY (if INCLUDE_vTaskSuspend == 1) !
 */
void mpu_task([[maybe_unused]] void *pvParameters) {
    int16_t ax, ay, az, gx, gy, gz;
    float fax, fay, faz, fgx, fgy, fgz;

    if (!init_success)
        vTaskDelete(NULL);

    for (;;) {
        mpudev.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        fax = abs(ax * accel_res);
        fay = abs(ay * accel_res);
        faz = abs(az * accel_res);
        fgx = abs(gx * gyro_res);
        fgy = abs(gy * gyro_res);
        fgz = abs(gz * gyro_res);

        xSemaphoreTake(mutex, portMAX_DELAY);

        if (internal.max_x < fax)
            internal.max_x = fax;
        if (internal.max_y < fay)
            internal.max_y = fay;
        if (internal.max_z < faz)
            internal.max_z = faz;

        internal.avg_x += fax;
        internal.avg_y += fay;
        internal.avg_z += faz;
        internal.rot_x += fgx;
        internal.rot_y += fgy;
        internal.rot_z += fgz;

        count++;

        xSemaphoreGive(mutex);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief Consume and display collected data.
 */
void pretty_print() {
    Data data = get_mpu();
    Serial.printf("[Max g force]\n"
                  "[x, y, z]\n"
                  "%f, %f, %f\n"
                  "[Average g force]\n"
                  "[x, y, z]\n"
                  "%f, %f, %f\n"
                  "[Average rotation]\n"
                  "[x, y, z]\n"
                  "%f, %f, %f\n",
                  data.max_x, data.max_y, data.max_z, data.avg_x, data.avg_y,
                  data.avg_z, data.rot_x, data.rot_y, data.rot_z);
    Serial.flush();
}

/**
 * @brief Task that consumes and displays collected data repeatedly.
 *        Die gracefully if sensor not initialised.
 */
void mpu_print([[maybe_unused]] void *pvParameters) {
    if (!init_success)
        vTaskDelete(NULL);

    for (;;) {
        pretty_print();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

} // namespace mpu