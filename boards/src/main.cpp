#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "logger.h"
#include "lora.h"
#include "memory.h"
#include "mpu.h"

struct Data {
  gps::Data gps_data{};
  mpu::Data mpu_data{};
  bmp::Data bmp_data{};

} data;
logger::Packet packet{};
void main_core0_loop_task(void *pvParameters) {
  //  logic
  for (;;) {
    String str = "Hello, World!";
    lora::lora_log(str);
    bmp::get_bmp(&data.bmp_data);
    packet.bmp_data = data.bmp_data;
    packet.mpu_data = data.mpu_data;
    packet.gps_data = data.gps_data;
    Serial.println(logger::serialize(packet));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(BAUD_RATE);
  Serial.println("--- ROCKET COMPUTER START ---");

  memory::init();
  gps::init();
  bmp::init();
  lora::init();
  mpu::init();

  xTaskCreatePinnedToCore(main_core0_loop_task, "main_core0_loop_task",
                          DEFAULT_TASK_SIZE, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(mpu::mpu_task, "mpu_task", DEFAULT_TASK_SIZE,
                          &data.mpu_data, 1, nullptr, 1);
  xTaskCreatePinnedToCore(gps::gps_task, "gps_task", DEFAULT_TASK_SIZE,
                          &data.gps_data, 1, nullptr, 1);

  memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
  memory::save_config();

#ifdef DEBUG
  // memory::print_debug();
#endif
}

void loop() {}
