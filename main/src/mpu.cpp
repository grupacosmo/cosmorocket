#include "freertos/task.h"
#include "mpu.h"

#include "Simple_MPU6050.h"

Simple_MPU6050 _mpu;

namespace mpu {

    void init() {



    }

    void get_mpu(void *pvParameters) {

        vTaskDelay(pdMS_TO_TICKS(500));

    }

}