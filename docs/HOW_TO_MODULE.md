# Jak robić moduł


## Krok po kroku
1. Przeanalizuj co masz zrobić w tasku
2. Obierz prostą nazwę dla twojej funkcjonalności (np. `bme`)
3. Stwórz plik `.hpp` w folderze `include/` o wybranej nazwie (np. `bme.hpp`)
4. Stwórz plik `.cpp` w folderze `src/` o wybranej nazwie (np. `bme.cpp`)
5. W pliku `.hpp` zadeklaruj metody, które pozwolą na korzystanie z twojej funkcjonalności
6. W pliku `.cpp` zaimplementuj metody zadeklarowane w pliku `.hpp`
7. W pliku `main.cpp` zaimportuj plik `.hpp` i użyj zadeklarowanych metod żeby przetestować czy wszystko działa

Jeżeli twój projekt nie jest częścią kodu głównego mikrokontrolera rakiety, stwórz w folderze 
[`projects/`](../projects/) folder o nazwie twojego projektu i tam umieść pliki.

## Przykład

To jest tylko przykład, jeżeli chcesz użyć np. Klas to możesz to zrobić. Pamiętaj że twoje 
rozwiązanie musi dobrze współgrać z resztą projektu.

`include/bme.hpp`
```cpp
// Taki magiczny cosiek zeby nie importowac pliku dwa razy
#pragma once

// Metody które będą dostępne z zewnątrz 
// dopisujemy prefix `bme_` żeby uniknąć konfliktów nazw z innymi naszymi bibliotekami
// Nie bój się zostawić komentarz który przekazuje ważne informacje:

/**
 * Wymaga wcześniejszego ustawienia `Wire.begin(...)`
 */
void bme_begin();

void bme_refresh();

/** Temperatura w °C */
int bme_getTemperature();

int bme_getHumidity();
```

`src/bme.cpp`
```cpp
// include naszego pliku `bme.hpp`
#include "bme.hpp"

// include potrzebnych bibliotek
#include <BME280_t.h>

BME280<> BMESensor;

// Kod dla metod z pliku `.hpp`:
void bme_begin() {
    BMESensor.begin();
}

void bme_refresh() {
    BMESensor.refresh();
}

float bme_getTemperature() {
    return BMESensor.temperature;
}

float bme_getHumidity() {
    return BMESensor.humidity;
}

// Jezeli potrzebujesz, możesz stworzyć metody, które nie są w pliku `bme.hpp`
// Pamiętaj że takie metody nie będą dostępne z zewnątrz tego pliku `.cpp`
```

`src/main.cpp`
```cpp
// Standardowy plik Arduino, jak znacie z Arduino IDE

#include <Arduino.h>
#include <Wire.h>
#include "bme.hpp"

void setup() {
    Serial.begin(9600);
    Wire.begin(12, 3);
    bme_begin(); // to nasza funkcja!
}

void loop() {
    bme_refresh();
    Serial.printf("temperature: %d \n", bme_getTemperature());
    Serial.printf("humidity:    %d \n", bme_getHumidity());
    delay(1000);
}
```

