#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "lora.h"
#include "memory.h"
#include "mpu.h"

struct Data {
  gps::Data gps_data{};
  mpu::Data mpu_data{};

} data;

void core0_task(void *pvParameters) {
  // xTaskCreate(gps::gps_task, "gps", DEFAULT_TASK_SIZE, &data.gps_data, 1,
  // nullptr);
  // gps::gps_task(&data.gps_data);
  // xTaskCreate(mpu::mpu_task, "mpu", DEFAULT_TASK_SIZE, &data.mpu_data, 8,
  // nullptr);
  Serial.println("Hello");
  // gps::print_debug(data.gps_data);
  // mpu::print_data(data.mpu_data);
  vTaskDelay(1000);
}

void core1_task(void *pvParameters) {
  xTaskCreate(bmp::get_bmp, "bmp", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  xTaskCreate(lora::lora_log, "lora", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  // gps::print_debug(data.gps_data);
  // mpu::print_data(data.mpu_data);
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(BAUD_RATE);
  Serial.println("--- ROCKET COMPUTER START ---");

  // memory::init();
  gps::init();
  // bmp::init();
  // lora::init();
  mpu::init();

  xTaskCreatePinnedToCore(core0_task, "core0_task", DEFAULT_TASK_SIZE, nullptr,
                          1, nullptr, 0);
  // xTaskCreatePinnedToCore(core1_task, "core1_task", DEFAULT_TASK_SIZE,
  // nullptr, 1, nullptr, 1);

  // memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
  // memory::save_config();
#ifdef DEBUG
  memory::print_debug();
#endif
}

void loop() {}
