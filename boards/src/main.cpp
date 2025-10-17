#include <Arduino.h>

#include "board_config.h"

void init_interfaces();

void setup() {
    Serial.begin(115200);
    Serial.println("Rocket initialisation started");

    init_interfaces();
}

void init_interfaces() { Wire.begin(I2C_SLAVE_SCL_PIN, I2C_SLAVE_SDA_PIN); }

void loop() {}
