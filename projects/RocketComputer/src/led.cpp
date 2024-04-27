void TaskBlinkLED(void *pvParameters) {
    pinMode(LED_BUILTIN, OUTPUT);
    for(;;) {
        digitalWrite(LED_BUILTIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500)); // delay for 500ms
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(500)); // delay for 500ms
    }
}