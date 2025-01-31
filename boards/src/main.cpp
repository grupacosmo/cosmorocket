#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "memory.h"
#include "mpu.h"
#include "our_lora.h"

struct Data {
  struct Data {
    gps::Data gps_data{};
    mpu::Data mpu_data{};
    bmp::Data bmp_data{};

  } data;

  void core0_task(void *pvParameters) {
    //  logic
    //  logger
    for (;;) {
      // lora::lora_log();
      bmp::get_bmp(&data.bmp_data);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.begin(BAUD_RATE);
    Serial.println("--- ROCKET COMPUTER START ---");

    // memory::init();
    // gps::init();
    // bmp::init();
    lora::init();
    // mpu::init();

    //   xTaskCreatePinnedToCore(core0_task, "core0_task", DEFAULT_TASK_SIZE,
    //   nullptr,
    //                           1, nullptr, 0);

    //   xTaskCreatePinnedToCore(mpu::mpu_task, "mpu_task", DEFAULT_TASK_SIZE,
    //                           &data.mpu_data, 1, nullptr, 1);
    //   xTaskCreatePinnedToCore(gps::gps_task, "gps_task", DEFAULT_TASK_SIZE,
    //                           &data.gps_data, 1, nullptr, 1);
    // xTaskCreatePinnedToCore(core1_task, "core1_task", DEFAULT_TASK_SIZE,
    // nullptr,
    //                         1, nullptr, 1);

    // memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
    // memory::save_config();
    // memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
    // memory::save_config();
#ifdef DEBUG
    // memory::print_debug();
#endif
  }

  void loop() {}
