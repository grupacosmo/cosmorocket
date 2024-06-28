#include <HardwareSerial.h>

namespace gps {
struct Time {
  uint8_t hours{};
  uint8_t minutes{};
  uint8_t seconds{};
};

struct Data {
  Time time{};
  double lat{};
  double lng{};
};

void init();
void gps_task(void *pvParameters);
Data get_gps_data();
bool data_is_available();

} // namespace gps
