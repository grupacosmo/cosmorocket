#define SPI3_MISO GPIO32
#define SPI3_MOSI GPIO15
#define SPI3_SCK GPIO25
#define SPI3_SS GPIO33


#include <Arduino.h>
#define SCK GPIO25
#define MISO GPIO32
#define MOSI GPIO15
#define SS 33

#include <Wire.h>

#include "bmp.h"
#include "gps.h"
#include "led.h"
#include "memory.h"
#include "sd_card.h"

std::uint8_t constexpr MY_SDA = 21;
std::uint8_t constexpr MY_SCL = 22;
unsigned long constexpr BAUD_RATE = 115200;
std::uint32_t constexpr DEFAULT_TASK_SIZE = 10000;

void setup() {
    Wire.begin(MY_SDA, MY_SCL);
    Serial.begin(BAUD_RATE);
    delay(500);
    Serial.println("--- ROCKET COMPUTER START ---");
    String penis = "Penis";
    sd::init();
    sd::write(penis, "/penis.file");
    Serial.println("--- DONE ---");

    // memory::init();
    // gps::init();
    // bmp::init();

    // xTaskCreate(led::blink_task, "blink", DEFAULT_TASK_SIZE, nullptr, 1,
    //             nullptr);
    // xTaskCreate(gps::gps_task, "gps", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
    // xTaskCreate(bmp::get_bmp, "bmp", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
    // xTaskCreate(bmp::print_data, "bmp print", DEFAULT_TASK_SIZE, nullptr, 1,
    //             nullptr);

    // memory::print_data();
    // memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
    // memory::save_config(memory::config);
}

void loop() {}
