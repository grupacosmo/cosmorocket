#include "logger.h"

namespace logger {

// Private
namespace {
int serialize_data(char *buf, size_t len, const Packet &packet) {
  return snprintf(
      // clang-format off
      buf, len,
      "%u,%u:%u:%u,%u,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f",
      packet.n, packet.gps_data.time.hours, packet.gps_data.time.minutes,
      packet.gps_data.time.seconds, packet.status, packet.bmp_data.temperature,
      packet.bmp_data.pressure, packet.bmp_data.altitude, packet.mpu_data.max.x,
      packet.mpu_data.max.y, packet.mpu_data.max.z, packet.mpu_data.avg.x,
      packet.mpu_data.avg.y, packet.mpu_data.avg.z, packet.mpu_data.rot.x,
      packet.mpu_data.rot.y, packet.mpu_data.rot.z, packet.gps_data.lat,
      packet.gps_data.lng);
}
// clang-format on
}  // namespace

String serialize(Packet &packet) {
  char buf[256];
  if (!serialize_data(&buf[0], 256, packet)) {
    Serial.println("Failed to serialize data, check buffer size");
    return String(
        "0,0,0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0."
        "0,0.0");
  }

  return String(buf);
}

}  // namespace logger
