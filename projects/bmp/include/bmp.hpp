#pragma once

//wymaga ustawienia Wire.begin(SDA_PIN, SCL_PIN) wcześniej
void bmp_begin();

//temperatura w Celsjuszach
float bmp_getTemp();

//Ciśnienie w paskalach
float bmp_getPress();

//przybliżona wysokość nad poziomem morza | w bmp.cpp można polepszyć szacunki zmieniając SEALEVELPRESSURE_HPA
float bmp_getAlt();