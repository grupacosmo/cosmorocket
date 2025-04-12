#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "memory.h"
#include "mpu.h"

#ifdef TBEAM
#include "lora.h"
#else
#include "lora-uart.h"
#endif

// sperate task pinned to core 1
void main_task(void *pvParameters) {
  logger::Packet packet;
  for (;;) {
    // logic here
    packet.bmp_data = bmp::get_data();
    packet.mpu_data = mpu::get_data();
    packet.gps_data = gps::get_data();
    String message = logger::serialize(packet);
    lora::lora_log(message);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(BAUD_RATE);
  Serial.println("--- ROCKET COMPUTER START ---");

  memory::init();
  gps::init();
  bmp::init();
  mpu::init();
  lora::init();
  // Pin mpu_task to core 0
  xTaskCreatePinnedToCore(mpu::mpu_task, "mpu", SIZE, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(gps::gps_task, "gps", SIZE, nullptr, 1, nullptr, 0);
  // Pin main_task to core 1
  xTaskCreatePinnedToCore(main_task, "main", SIZE, nullptr, 1, nullptr, 1);

  Serial.println("--- SETUP FINISHED. ---");
}

void loop() {}
