#include "logger.h"

namespace logger {

// Private
namespace {
constexpr size_t SERIALIZE_PACKET_SIZE = 1024;
char buf[SERIALIZE_PACKET_SIZE];

int serialize_data(char *buf, size_t len, const Packet &packet) {
  return snprintf(
      // clang-format off
      buf, len,
      "%u,%u:%u:%u,%u,%0.4f,%0.4f,%0.4f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f",
      packet.n, packet.gps_data.time.hours, packet.gps_data.time.minutes, 
      packet.gps_data.time.seconds, packet.status,
      packet.bmp_data.temperature, packet.bmp_data.pressure, packet.bmp_data.altitude,
      packet.mpu_data.acc_max.x, packet.mpu_data.acc_max.y, packet.mpu_data.acc_max.z,
      packet.mpu_data.acc_avg.x, packet.mpu_data.acc_avg.y, packet.mpu_data.acc_avg.z,
      packet.mpu_data.gyro_max.x, packet.mpu_data.gyro_max.y, packet.mpu_data.gyro_max.z,
      packet.mpu_data.gyro_avg.x, packet.mpu_data.gyro_avg.y, packet.mpu_data.gyro_avg.z,
      packet.mpu_data.rot_avg.x, packet.mpu_data.rot_avg.y, packet.mpu_data.rot_avg.z, packet.mpu_data.rot_avg.w,
      packet.gps_data.lat, packet.gps_data.lng);
}
// clang-format on
}  // namespace

const char *serialize(Packet &packet) {
  if (!serialize_data(&buf[0], SERIALIZE_PACKET_SIZE, packet)) {
    Serial.println("Failed to serialize data, check buffer size");
    serialize_data(&buf[0], SERIALIZE_PACKET_SIZE, Packet{});
  }

  return buf;
}

}  // namespace logger
