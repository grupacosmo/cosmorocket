namespace bmp {
// wymaga ustawienia Wire.begin(SDA_PIN, SCL_PIN) wcześniej
void init();

struct Data {
    float temperature;
    float pressure;
    float altitude;

    float get_as_hpa();
};

inline Data data;

void get_bmp(void *pvParameters);
void print_data(void *pvParameters);
Data measurements();
void pretty_print(Data);
} // namespace bmp