#include "button.h"

namespace button {

void button_task(void* pvParameters)
{
    uint32_t rx_msg;
    for (;;) {
        if (xQueueReceive(xQueue, &rx_msg, portMAX_DELAY)) {
            Serial.println("Button pressed!");
        }
    }
}
} // namespace button