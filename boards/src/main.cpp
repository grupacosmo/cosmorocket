#include <Arduino.h>
#include <I2Cdev.h>

#include "board_config.h"

void initInterfaces();

void setup() {
    initInterfaces();

    Serial.println("Rocket initialisation started");
}

void initInterfaces() {
    Serial.begin(115200);
    Wire.begin(board_config::I2C_SLAVE_SCL_PIN,
               board_config::I2C_SLAVE_SDA_PIN);
}

void loop() {}
