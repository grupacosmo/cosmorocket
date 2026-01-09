#include "freertos/FreeRTOS.h"
#include "i2c.h"

static const char *TAG = "rocket";

extern "C" void app_main(void) {
    i2c::init();

    while (true) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
