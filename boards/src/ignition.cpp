#include "ignition.h"

#include <Arduino.h>

#include "board_config.h"

namespace ignition {

void init() {
  pinMode(P1, OUTPUT);
  pinMode(P2, OUTPUT);
  pinMode(P3, OUTPUT);
  digitalWrite(P1, LOW);
  digitalWrite(P2, LOW);
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

  Serial.println("[IGNITION] Initialized ignition system.");
}

void fire(uint8_t pin) {
  if (pin) {
    digitalWrite(P1, HIGH);
    Serial.println("[IGNITION] Fired P1.");
    vTaskDelay(pdMS_TO_TICKS(5000));  // Wait for 5 second
    digitalWrite(P1, LOW);
  }
}

}  // namespace ignition
