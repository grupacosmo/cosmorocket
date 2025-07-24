#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <cstdint>

namespace ignition {

extern TaskHandle_t camPtr;
extern int duration;  // Duration for the camera task in milliseconds

void init();
void fire(uint8_t pin);
void cam_task(void *pvParameters);
}  // namespace ignition