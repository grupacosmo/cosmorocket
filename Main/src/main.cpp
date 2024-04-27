#include <Arduino.h>

#include "button.h"
#include "interrupts.h"
#include "led.h"

QueueHandle_t xQueue;

void setup() {
  Serial.begin(115200);
  Serial.println("--- Starting... ---");

  xQueue = xQueueCreate(10, sizeof(uint32_t));  // create xQueue
  interrupts::setup();

  //------ tutorial of how to setup a task
  xTaskCreate(                  /* Command to create a task*/
              led::blink_task,  /* Task function name */
              "LOLOLOLOLBLINK", /* String with name of task. */
              10000, /* Stack size in bytes. - leave it 10000 for now*/
              NULL,  /* Parameter passed as input - leave NULL for now*/
              1,     /* Priority of the task.- leave 1 for now*/
              NULL); /* Task handle. - leave NULL if not neccesary */

  //---------- below is the normal declaration
  if (xQueue != NULL)
    xTaskCreate(button::button_task, "TaskButton", 10000, NULL, 1, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
