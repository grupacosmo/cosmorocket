#include "ignition.h"

#include <Arduino.h>

#include "board_config.h"

namespace ignition {

void init() {
  pinMode(P2_PARACHUTE, OUTPUT);
  pinMode(P3, OUTPUT);
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

  Serial.println("[IGNITION] Initialized ignition system.");
}

void fire(uint8_t pin) {
  if (pin) {
    digitalWrite(pin, HIGH);
    Serial.println("[IGNITION] Fired ignition pin: " + String(pin));
  }
}

}  // namespace ignition
