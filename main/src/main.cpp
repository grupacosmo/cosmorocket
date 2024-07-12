#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "memory.h"

std::uint8_t constexpr SDL_PIN = SDA_PIN_;
std::uint8_t constexpr SCL_PIN = SCL_PIN_;
unsigned long constexpr BAUD_RATE = 115200;
std::uint32_t constexpr DEFAULT_TASK_SIZE = 10000;

void setup() {
    Wire.begin(SDL_PIN, SCL_PIN);
    Serial.begin(BAUD_RATE);
    Serial.println("--- ROCKET COMPUTER START ---");

    memory::init();
    gps::init();
    bmp::init();

    xTaskCreate(led::blink_task, "blink", DEFAULT_TASK_SIZE, nullptr, 1,
                nullptr);
    xTaskCreate(gps::gps_task, "gps", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
    xTaskCreate(bmp::get_bmp, "bmp", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
    xTaskCreate(bmp::print_data, "bmp print", DEFAULT_TASK_SIZE, nullptr, 1,
                nullptr);

    memory::print_data();
    memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
    memory::save_config(memory::config);
}

void loop() {}
