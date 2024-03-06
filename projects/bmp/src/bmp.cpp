#include "bmp.hpp"

//korzystanie z Adafruit BMP280 Library by Adafruit
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// ciśnienie powietrza na poziomie morza w hPa, używane w bmp_getAlt
// 1013.25 jest ogólną średnią 
// 1019.91 jest wartością lokalną dla Krakowa w momencie pisania tego
#define SEALEVELPRESSURE_HPA (1019.91)

Adafruit_BMP280 bmp;

//wymaga ustawienia Wire.begin(SDA_PIN, SCL_PIN) wcześniej
void bmp_begin(){
    if (!bmp.begin(0x76)) {  
    Serial.println("Nie znaleziono odpowiedniego sensora BMP280, sprawdz okablowanie!");
    while (1);
  }
}

//temperatura w Celsjuszach
float bmp_getTemp(){
    return bmp.readTemperature();
}

//Ciśnienie w paskalach
float bmp_getPress(){
    return bmp.readPressure();
}


//przybliżona wysokość nad poziomem morza
float bmp_getAlt(){
    return bmp.readAltitude(SEALEVELPRESSURE_HPA);
}