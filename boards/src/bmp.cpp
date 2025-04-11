#include "bmp.h"

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

namespace bmp {

// Private
namespace {
// 1019.91 is avg Pressure in Cracow
constexpr float SEALEVELPRESSURE_HPA = 1019.91;

constexpr uint8_t CHIP_BME = 0x58;
//constexpr uint8_t CHIP_BME = 0x60;   // ALT 0x58
constexpr uint8_t CHIP_ADDR = 0x76;  // ALT 0x77

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
  bool success = bmp_obj.begin(CHIP_ADDR, CHIP_BME);

  if (!success) {
    Serial.println("Viable sensor BMP280 not found, check wiring!");
  }
}

void get_bmp(void *pvParameters) {
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(500));
    float temperature = bmp_obj.readTemperature();
    float pressure = bmp_obj.readPressure();
    if (pressure < 0) { // if garbage data
      data = Data{};
      continue;
    }
    
    // default values -> no measurement taken -> posible recent power loss -> the sensor is not initialized. We need to reinitialize it
    if (fabs(temperature - 24.1500) < 0.001
          && fabs(pressure - 74296.8906) < 0.001) {
      Serial.println("Reinitialising BMP");
      bmp_obj.begin(CHIP_ADDR, CHIP_BME);
      
      continue;
    }
    data = Data{.temperature = temperature,
                .pressure = pressure,
                .altitude = bmp_obj.readAltitude(SEALEVELPRESSURE_HPA)};
#ifdef DEBUG
      print_debug(&data);
#endif
  }
}

Data get_data() {
  return data;
}

}  // namespace bmp
