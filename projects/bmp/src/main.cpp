#include <Arduino.h>
#include <Wire.h>
#include "bmp.hpp"


void setup() {
  Wire.begin(23,19);
  Serial.begin(9600);
  bmp::begin();
}

void loop() {
  bmp::pretty_print();
  delay(2000);
}