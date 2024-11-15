#include "bmp.h"

namespace bmp {

// Private
namespace {
// 1019.91 is avg Pressure in Cracow
constexpr float SEALEVELPRESSURE_HPA = 1019.91;

constexpr uint8_t CHIP_BME = 0x60;   // ALT 0x58
constexpr uint8_t CHIP_ADDR = 0x76;  // ALT 0x77

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

void init(Adafruit_BMP280& bmp) {
  bool success = bmp.begin(CHIP_ADDR, CHIP_BME);

  if (!success) {
    Serial.println("Viable sensor BMP280 not found, check wiring!");
  }
}

Data read(Adafruit_BMP280& bmp) {
  return Data{
    .temperature = bmp.readTemperature(),
    .pressure = bmp.readPressure(),
    .altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)
  };
}

}  // namespace bmp
