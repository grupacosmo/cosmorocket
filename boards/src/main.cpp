#include <HardwareSerial.h>

#include "barometer.h"
#include "board_config.h"
#include "i2c.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Rocket initialisation started");

    i2c::init();
    barometer::init();
}

void loop() {
    delay(500);
    float air_pressure, temperature;
    barometer::getData(air_pressure, temperature);

    Serial.print(air_pressure);
    Serial.print(" Pa, ");
    Serial.print(temperature);
    Serial.println(" C");
}
