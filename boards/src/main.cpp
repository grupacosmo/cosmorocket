#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "lora.h"
#include "memory.h"
#include "mpu.h"
#include "display.h"

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(BAUD_RATE);
  Serial.println("--- ROCKET COMPUTER START ---");

  memory::init();
  display::init();
  gps::init();
  bmp::init();
  //lora::init();
  mpu::init();

  xTaskCreate(led::blink_task, "blink", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  vTaskDelay(pdMS_TO_TICKS(200));
  //xTaskCreate(gps::gps_task, "gps", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  xTaskCreate(bmp::get_bmp, "bmp", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  //xTaskCreate(lora::lora_log, "lora", DEFAULT_TASK_SIZE, NULL, 1, NULL);
  xTaskCreate(mpu::mpu_task, "mpu", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  xTaskCreate(display::display_task, "display", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);

  memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
  memory::save_config();
#ifdef DEBUG
  memory::print_debug();
#endif
}

void loop() {}
