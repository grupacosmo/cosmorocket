#include "interrupts.h"

namespace interrupts {

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_isr, FALLING);
}

void IRAM_ATTR button_isr() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  uint32_t isr_msg = 1;
  xQueueSendFromISR(xQueue, &isr_msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}
}  // namespace interrupts
