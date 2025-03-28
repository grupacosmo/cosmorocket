#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "lora.h"
#include "lora-uart.h"
#include "memory.h"
#include "mpu.h"

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(BAUD_RATE);
  Serial.println("--- ROCKET COMPUTER START ---");

  memory::init();
  gps::init();
  bmp::init();
#ifdef TBEAM
  lora::init();
#else
  lora_uart::init();
#endif
  mpu::init();

  xTaskCreate(led::blink_task, "blink", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  xTaskCreate(gps::gps_task, "gps", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  xTaskCreate(bmp::get_bmp, "bmp", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
#ifdef TBEAM
  xTaskCreate(lora::lora_task, "lora", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
#else
  xTaskCreate(lora_uart::lora_log, "lora", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
#endif
  xTaskCreate(mpu::mpu_task, "mpu", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);

  memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
  memory::save_config();
#ifdef DEBUG
  memory::print_debug();
#endif
}

void loop() {}
