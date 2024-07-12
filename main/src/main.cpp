#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "memory.h"
#include "lora.h"

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.begin(BAUD_RATE);
    Serial.println("--- ROCKET COMPUTER START ---");

    memory::init();
    gps::init();
    bmp::init();
    lora::init();

    xTaskCreate(led::blink_task, "blink", DEFAULT_TASK_SIZE, nullptr, 1,
                nullptr);
    xTaskCreate(gps::gps_task, "gps", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
    xTaskCreate(bmp::get_bmp, "bmp", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
    xTaskCreate(bmp::print_data, "bmp print", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
    xTaskCreate(lora::lora_log, "lora", DEFAULT_TASK_SIZE, NULL, 1, NULL);

    memory::print_data();
    memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
    memory::save_config(memory::config);
}

void loop() {}
