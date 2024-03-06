#include <Arduino.h>
#include <Wire.h>
#include "bmp.hpp"


void setup() {
  Wire.begin(23,19);
  Serial.begin(9600);
  bmp_begin();
}


void loop() {
  Serial.print("Temperature = ");
  Serial.print(bmp_getTemp());
  Serial.println(" *C");
  
  Serial.print("Pressure = ");
  Serial.print(bmp_getPress()/ 100.0F);
  Serial.println(" hPa");
  
  Serial.print("Approx. Altitude = ");
  Serial.print(bmp_getAlt());
  Serial.println(" m");
  
  Serial.println();
  delay(2000);
}
