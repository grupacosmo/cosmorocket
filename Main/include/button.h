#include <Arduino.h>
extern QueueHandle_t xQueue;

namespace button {

void button_task(void *pvParameters);
} // namespace button