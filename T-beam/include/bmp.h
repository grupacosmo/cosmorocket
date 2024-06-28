namespace bmp {
// wymaga ustawienia Wire.begin(SDA_PIN, SCL_PIN) wcześniej
void init();

struct Data {
  float temperature;
  float pressure;
  float altitude;

  float get_as_hpa();
};

inline Data global_bmp;

void get_bme(void* pvParameters);
Data measurements();
void pretty_print();
}  // namespace bmp