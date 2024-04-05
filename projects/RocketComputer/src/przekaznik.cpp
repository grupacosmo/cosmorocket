#include <Arduino.h>

void setup(){
    pinMode(26, OUTPUT);

}

void loop(){
    
    digitalWrite(26, HIGH);
    delay(1000);
    digitalWrite(26, LOW);
    delay(1000);

}