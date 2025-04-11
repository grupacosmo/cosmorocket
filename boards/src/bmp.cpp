#include "bmp.h"

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

namespace bmp {

// Private
namespace {
// 1019.91 is avg Pressure in Cracow
constexpr float SEALEVELPRESSURE_HPA = 1019.91;

Adafruit_BMP280 bmp_obj;
Data data;

void print_debug(const Data &data) {
  Serial.print("[Temperature] ");
  Serial.print(data.temperature);
  Serial.println("°C");

  Serial.print("[Pressure] ");
  Serial.print(data.pressure / 100.f);
  Serial.println("hPa");

  Serial.print("[Altitude] ");
  Serial.print(data.altitude);
  Serial.println("m");
}
}  // namespace

void init() {
  bool success =
      bmp_obj.begin(0x76, 0x60) ||  // Primary address, primary chip ID
      bmp_obj.begin(0x76, 0x58) ||  // Primary address, alternative chip ID
      bmp_obj.begin(0x77, 0x60) ||  // Alternative address, primary chip ID
      bmp_obj.begin(0x77, 0x58);    // Alternative address, alternative chip ID

  if (!success) {
    Serial.println("Viable sensor BMP280/BME280 not found, check wiring!");
  } else {
    Serial.println("BMP280/BME280 sensor initialized successfully.");
  }
}

void get_bmp(void *pvParameters) {
  for (;;) {
    if (bmp_obj.sensorID() != 0) {  // TODO Test should be 0 if not inited
      data = Data{.temperature = bmp_obj.readTemperature(),
                  .pressure = bmp_obj.readPressure(),
                  .altitude = bmp_obj.readAltitude(SEALEVELPRESSURE_HPA)};
#ifdef DEBUG
      print_debug(data);
#endif
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

Data get_data() { return data; }

}  // namespace bmp
