#include "bmp.h"

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include "display.h"

namespace bmp {

// Private
namespace {
// 1019.91 is avg Pressure in Cracow
constexpr float SEALEVELPRESSURE_HPA = 1019.91;

Adafruit_BMP280 bmp_obj;
Data data;
}  // namespace

void print_debug() {
  Adafruit_SSD1306 *disp = display::display_get();

  disp->print("Temperature: ");
  disp->print(data.temperature);
  disp->println(" C");

  disp->print("Pressure: ");
  disp->print(data.pressure / 100.f);
  disp->println("hPa");

  /*disp->print("[Altitude] ");
  disp->print(data.altitude);
  disp->println("m");*/
}

void init() {
  bool success = bmp_obj.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);

  if (!success) {
    Serial.println("Viable sensor BMP280 not found, check wiring!");
  }
}

void get_bmp(void *pvParameters) {
  for (;;) {
    if (bmp_obj.sensorID() != 0) {  // TODO Test should be 0 if not inited
      data = Data{.temperature = bmp_obj.readTemperature(),
                  .pressure = bmp_obj.readPressure(),
                  .altitude = bmp_obj.readAltitude(SEALEVELPRESSURE_HPA)};
/*#ifdef DEBUG
      print_debug(data);
#endif*/
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

}  // namespace bmp
