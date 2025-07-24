#include "camera.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include "board_config.h"

namespace camera {
namespace {
TimerHandle_t camera_test_timer;
void timer_callback(TimerHandle_t timer) {
  digitalWrite(P1_CAMERA, LOW);
  Serial.println("[CAMERA] Camera turn off.");
  xTimerStop(timer, 0);
}
}  // namespace

void init() {
  Serial.println("[CAMERA] Initialized camera.");
  pinMode(P1_CAMERA, OUTPUT);
  digitalWrite(P1_CAMERA, LOW);
  camera_start(10000);
}

void camera_start(int delay_ms) {
  camera_test_timer = xTimerCreate("camera_timer", pdMS_TO_TICKS(delay_ms),
                                   pdFALSE, (void *)0, timer_callback);
  xTimerStart(camera_test_timer, 0);
  digitalWrite(P1_CAMERA, HIGH);
  Serial.println("[CAMERA] Camera has started.");
}

}  // namespace camera
