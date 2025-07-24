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
  Serial.println("[CAMERA] Camera test completed.");
  xTimerStop(timer, 0);
}
}  // namespace

void init() {
  Serial.println("[CAMERA] Initialized camera.");
  pinMode(P1_CAMERA, OUTPUT);
  digitalWrite(P1_CAMERA, LOW);
  camera_test_timer = xTimerCreate("camera_timer", pdMS_TO_TICKS(10000),
                                   pdFALSE, (void *)0, timer_callback);
}

void test() {
  xTimerStart(camera_test_timer, 0);
  digitalWrite(P1_CAMERA, HIGH);
  Serial.println("[CAMERA] Camera test started.");
}

void start_camera() {
  if (digitalRead(P1_CAMERA) == LOW) {
    digitalWrite(P1_CAMERA, HIGH);
  }
}

void stop_camera() { digitalWrite(P1_CAMERA, LOW); }

}  // namespace camera
