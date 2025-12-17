#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "rocket";

extern "C" void app_main(void) {
    while (true) {
        ESP_LOGI(TAG, "Hello World!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
