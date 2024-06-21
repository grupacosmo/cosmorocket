#include <Arduino.h>

#include "gps.h"
#include "led.h"

void setup() {
  Serial.begin(115200);
  Serial.println("--- Starting the rocket... ---");
  gps::init();
  xTaskCreate(led::blink_task, "blink", 10000, NULL, 1, NULL);
  xTaskCreate(gps::gps_loop, "gps", 10000, NULL, 1, NULL);
}

void loop() {}
