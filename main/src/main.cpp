#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "gps.h"
#include "led.h"
#include "memory.h"

#define SDA 21
#define SCL 22
#define BAUD_RATE 115200
#define DEFAULT_TASK_SIZE 10000

/// Log data from sensors
/// For now it only logs gps data to console, eventually
/// it should write the data from all components to a disk
void log(void *pvParameters) {
    for (;;) {
        if (!gps::data_is_available()) {
            Serial.println("GPS data is unavailable.");
        } else {
            gps::Data gps_data = gps::get_gps_data();
            Serial.printf("Lat: %.6f Long: %.6f Time: %02d:%02d:%02d\n",
                          gps_data.lat, gps_data.lng, gps_data.time.hours,
                          gps_data.time.minutes, gps_data.time.seconds);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
    Wire.begin(SDA, SCL);
    Serial.begin(BAUD_RATE);
    Serial.println("--- ROCKET COMPUTER START ---");

    memory::init();
    gps::init();
    bmp::init();

    xTaskCreate(led::blink_task, "blink", 10000, NULL, 1, NULL);
    xTaskCreate(gps::gps_task, "gps", 10000, NULL, 1, NULL);
    xTaskCreate(log, "log", 10000, NULL, 1, NULL);
    xTaskCreate(bmp::get_bmp, "bmp", DEFAULT_TASK_SIZE, NULL, 1, NULL);
    xTaskCreate(bmp::print_data, "bmp print", DEFAULT_TASK_SIZE, NULL, 1, NULL);

    memory::print_data();
    memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
    memory::save_config(memory::config);
}

void loop() {}
