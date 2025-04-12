#include "bmp.h"

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

namespace bmp {

// Private
namespace {
// 1019.91 is avg Pressure in Cracow
constexpr float SEALEVELPRESSURE_HPA = 1019.91;
constexpr float DEFAULT_BMP_TEMPERATURE = 24.1500;
constexpr float DEFAULT_BMP_PRESSURE = 74296.8906;

Adafruit_BMP280 bmp_obj;

void print_debug(const Data &data) {
  Serial.println("[BMP280]:");
  Serial.print("---------[Temperature] ");
  Serial.print(data.temperature);
  Serial.println("°C");

  Serial.print("---------[Pressure] ");
  Serial.print(data.pressure / 100.f);
  Serial.println("hPa");

  Serial.print("---------[Altitude] ");
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
    Serial.println("BMP280 initialized successfully.");
  }
}

Data get_data() {
  Data data;
  float temperature = bmp_obj.readTemperature();
  float pressure = bmp_obj.readPressure();
  if (pressure < 0) {  // if garbage data
    return Data{};
  }

  // default values -> no measurement taken -> posible recent power loss ->
  // the sensor is not initialized. We need to reinitialize it
  if (fabs(temperature - DEFAULT_BMP_TEMPERATURE) < 0.001 &&
      fabs(pressure - DEFAULT_BMP_PRESSURE) < 0.001) {
    Serial.println("Reinitialising BMP");
    init();
    return Data{};
  }
  if (bmp_obj.sensorID() != 0) {  // TODO Test should be 0 if not inited
    data = Data{.temperature = bmp_obj.readTemperature(),
                .pressure = bmp_obj.readPressure(),
                .altitude = bmp_obj.readAltitude(SEALEVELPRESSURE_HPA)};
#ifdef DEBUG
    print_debug(data);
#endif
  }
  return data;
}

}  // namespace bmp
