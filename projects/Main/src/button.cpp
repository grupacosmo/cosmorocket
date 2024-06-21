#include "button.h"

namespace button {

void button_task(void* pvParameters) {
  Serial.println("Button pressed!");
  vTaskDelete(NULL);
}
}  // namespace button