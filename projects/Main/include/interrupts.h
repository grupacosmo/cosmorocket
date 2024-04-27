#include <Arduino.h>
#define BUTTON_PIN 0  // Change this to your button pin

extern QueueHandle_t xQueue;

namespace interrupts {

void setup();
void IRAM_ATTR button_isr();
}  // namespace interrupts
