#include "ignition.h"

#include <Arduino.h>

#include "board_config.h"

namespace ignition {

TaskHandle_t camPtr = nullptr;
int duration = 10000;  // Default duration for the camera task

void cam_task(void *pvParameters) {
  digitalWrite(P1_CAMERA, HIGH);
  Serial.print(duration);
  vTaskDelay(pdMS_TO_TICKS(duration));  // Wait for the specified duration
  digitalWrite(P1_CAMERA, LOW);
  Serial.println("[IGNITION] Camera task completed.");
  vTaskDelete(nullptr);  // Delete the current task
}

void init() {
  pinMode(P1_CAMERA, OUTPUT);
  pinMode(P2_PARACHUTE, OUTPUT);
  pinMode(P3, OUTPUT);
  digitalWrite(P1_CAMERA, LOW);
  digitalWrite(P2_PARACHUTE, LOW);
  digitalWrite(P3, LOW);
  pinMode(PT1, INPUT);
  pinMode(PT2, INPUT);
  pinMode(PT3, INPUT);

  if (digitalRead(PT1) == LOW) {
    Serial.println("[IGNITION] PT1 is LOW, ignition system is not ready.");
  }
  if (digitalRead(PT2) == LOW) {
    Serial.println("[IGNITION] PT2 is LOW, ignition system is not ready.");
  }
  if (digitalRead(PT3) == LOW) {
    Serial.println("[IGNITION] PT3 is LOW, ignition system is not ready.");
  }

  xTaskCreatePinnedToCore(cam_task, "cam", 1024, nullptr, 1, &camPtr, 1);
  Serial.println("[IGNITION] Initialized ignition system.");
}

void fire(uint8_t pin) {
  if (pin) {
    digitalWrite(pin, HIGH);
    Serial.println("[IGNITION] Fired ignition pin: " + String(pin));
  }
}

}  // namespace ignition
