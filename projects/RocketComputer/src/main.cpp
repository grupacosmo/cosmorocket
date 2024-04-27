






#include <Arduino.h>
#include <FreeRTOS.h>

#define BUTTON_PIN 0 // Change this to your button pin

QueueHandle_t xQueue;

void IRAM_ATTR button_isr() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t isr_msg = 1;
    xQueueSendFromISR(xQueue, &isr_msg, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void TaskButton(void *pvParameters) {
    uint32_t rx_msg;
    for (;;) {
        if (xQueueReceive(xQueue, &rx_msg, portMAX_DELAY)) {
            // Button press detected, do something here
        }
    }
}

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_isr, FALLING);

    xQueue = xQueueCreate(10, sizeof(uint32_t));
    if (xQueue != NULL) {
        xTaskCreate(TaskButton, "TaskButton", 10000, NULL, 1, NULL);
    }
        xTaskCreate(
        TaskBlinkLED,          /* Task function. */
        "TaskBlinkLED",        /* String with name of task. */
        10000,                  /* Stack size in bytes. */
        NULL,                   /* Parameter passed as input of the task */
        1,                      /* Priority of the task. */
        NULL);                  /* Task handle. */
}

void loop() {
    // put your main code here, to run repeatedly:
}




