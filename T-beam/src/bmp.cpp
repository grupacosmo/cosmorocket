#include "bmp.h"

// korzystanie z Adafruit BMP280 Library by Adafruit
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

// ciśnienie powietrza na poziomie morza w hPa, altitude
// 1013.25 jest ogólną średnią
// 1019.91 jest wartością lokalną dla Krakowa w momencie pisania tego
#define SEALEVELPRESSURE_HPA (1019.91)

namespace bmp {

Adafruit_BMP280 bmp_obj;

// wymaga ustawienia Wire.begin(SDA_PIN, SCL_PIN) wcześniej
void init() {
  if (!bmp_obj.begin(0x76)) {
    Serial.println("Viable sensor BMP280 not found, check wiring!");
    while (1);
  }
}

void get_bme(void* pvParameters) {
  for (;;) {
    global_bmp = measurements();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

Data measurements() {
  Data check;
  // temperatura w Celsjuszach
  check.temperature = bmp_obj.readTemperature();
  // Ciśnienie w paskalach
  check.pressure = bmp_obj.readPressure();
  // przybliżona wysokość nad poziomem morza w metrach
  check.altitude = bmp_obj.readAltitude(SEALEVELPRESSURE_HPA);
  return check;
};

void pretty_print() {
  Data exp = measurements();
  Serial.print("Temperature = ");
  Serial.print(exp.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(exp.get_as_hpa());
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(exp.altitude);
  Serial.println(" m");

  Serial.println();
}

float Data::get_as_hpa() { return bmp_obj.readPressure() / 100.0F; }
}  // namespace bmp