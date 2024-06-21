#pragma once
namespace bmp {
// wymaga ustawienia Wire.begin(SDA_PIN, SCL_PIN) wcześniej
void begin();

struct Data {
  float temperature;
  float pressure;
  float altitude;

  float get_as_hpa();
};

Data measurements();
void pretty_print();
}  // namespace bmp