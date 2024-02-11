#include <Arduino.h>

int x;

void setup() {
  Serial.begin(9600);
  x = 0;
}

void loop() {
  Serial.printf("X = %d \n", x);
  x = x + 1;
  delay(100);
}