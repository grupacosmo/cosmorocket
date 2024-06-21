#include <Arduino.h>

#include "bmp.hpp"
#include "button.h"
#include "interrupts.h"
#include "led.h"

QueueHandle_t xQueue;

void setup() {
  Serial.begin(115200);
  Serial.println("--- Starting the rocket... ---");

  xQueue = xQueueCreate(10, sizeof(uint32_t));  // create xQueue
  interrupts::setup();
  xTaskCreate(led::blink_task, "blink", 10000, NULL, 1, NULL);

  bmp::Data globalData[10];

  void bmp_task() {
    for (;;) {
      bmp::Data tempData[10];
      for (int i = 0; i < 10; i++) {
        tempData[i] = bmp::measure();
        vTaskDelay(pdMS_TO_TICKS(100));
      }
      // Copy tempData to globalData
      memcpy(globalData, tempData, sizeof(globalData));
    }
  }

  xTaskCreate(led::blink_task, "blink", 10000, NULL, 1, NULL);
  if (xQueue != NULL)
    xTaskCreate(button::button_task, "button", 10000, NULL, 1, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
