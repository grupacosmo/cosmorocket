#include "led.h"

namespace led {
void setup() {}

void blink_task(void *pvParameters) {
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) {
        digitalWrite(LED_BUILTIN, HIGH);
        vTaskDelay(
            pdMS_TO_TICKS(500)); // delay for 500ms without blocking the CPU
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(500)); // delay for 500ms
    }
}
} // namespace led