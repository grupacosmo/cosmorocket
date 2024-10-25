#include "mpu.h"
#include "MPU6050.h"

MPU6050 mpudev;

namespace mpu {

    /**
     * @brief Inicjalizacja mpu
     */
    void init() {

        // W parametrach podaje się zakres pomiarów
        mpudev.initialize(/*MPU6050_ACCEL_FS_2, MPU6050_GYRO_FS_250*/);
        // Kalibracja?
        // mpudev.CalibrateAccel(10);
        // mpudev.CalibrateGyro(10);

        // Ewentualnie ustawienie predefiniowanych offsetow
        // zdobytych przez uruchomienie mpu_zero w przykladowych
        // programach
        // mpudev.setXAccelOffset(0);
        // mpudev.setYAccelOffset(0);
        // mpudev.setZAccelOffset(0);
        // mpudev.setXGyroOffset(0);
        // mpudev.setYGyroOffset(0);
        // mpudev.setZGyroOffset(0);

        if(!mpudev.testConnection()) {
            // die
            Serial.println("Connection to mpu failed");
            return;
        }

        init_success = true;

        accel_res = mpudev.get_acce_resolution() / 2;
        gyro_res = mpudev.get_gyro_resolution() / 2;

        Serial.println("accel_res");
        Serial.println(accel_res, 16);
        Serial.println("gyro_res");
        Serial.println(gyro_res, 16);

    }

    /**
     * @return Maksymalne przeciazenie, srednie przeciazenie, sredni obrot
     */
    Data get_mpu() {

        Data dane {
            .max_x = internal.max_x,
            .max_y = internal.max_y,
            .max_z = internal.max_z,
            .avg_x = internal.avg_x / count, // Ewentualnie mozna zaimplementowac
            .avg_y = internal.avg_y / count, // moving average (dunno czy w tym przypadku
            .avg_z = internal.avg_z / count, // jest to dobre rozwiazanie)
            .rot_x = internal.rot_x / count,
            .rot_y = internal.rot_y / count,
            .rot_z = internal.rot_z / count
        };

        count = 0;
        internal = {};

        return dane;

    }

    /**
     * @brief Funkcja wywoływana przez FreeRTOS jako task, zbiera dane z czujnika
     */
    void mpu_task([[maybe_unused]] void *pvParameters) {

        int16_t ax, ay, az, gx, gy, gz;
        float fax, fay, faz, fgx, fgy, fgz;

        if(!init_success) vTaskDelete(NULL);

        for(;;) {

            mpudev.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

            fax = abs(ax * accel_res);
            fay = abs(ay * accel_res);
            faz = abs(az * accel_res);
            fgx = abs(gx * gyro_res);
            fgy = abs(gy * gyro_res);
            fgz = abs(gz * gyro_res);

            if(internal.max_x < fax)
                internal.max_x = fax;

            if(internal.max_y < fay)
                internal.max_y = fay;

            if(internal.max_z < faz)
                internal.max_z = faz;

            internal.avg_x += fax;
            internal.avg_y += fay;
            internal.avg_z += faz;
            internal.rot_x += fgx;
            internal.rot_y += fgy;
            internal.rot_z += fgz;

            count++;

            vTaskDelay(pdMS_TO_TICKS(100));

        }

    }

    /**
     * @brief Wyswietlenie obecnie zebranych danych
     */
    void mpu_print([[maybe_unused]] void *pvParameters) {

        if(!init_success) vTaskDelete(NULL);

        for(;;) {
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
                      data.max_x, data.max_y, data.max_z,
                      data.avg_x, data.avg_y, data.avg_z,
                      data.rot_x, data.rot_y, data.rot_z);
        Serial.flush();
        vTaskDelay(pdMS_TO_TICKS(1000));
        }

    }

} // namespace mpu