#include "logger.h"

namespace logger {

// Private
namespace {
char buf[SERIALIZE_PACKET_SIZE];

bool check_write(int written, int remaining_len) {
  if (written < 0) {
    return false;
  }
  if (written >= remaining_len) {
    return false;
  }
  return true;
}

}  // namespace

const char *serialize(Packet &packet) {
  static uint16_t n = 0;
  n++;

  int offset = 0;
  int remaining_len = SERIALIZE_PACKET_SIZE;
  int written = 0;

  written = snprintf(buf + offset, remaining_len, "%u,%u,%u,%u,%u,", n,
                     packet.gps_data.time.hours, packet.gps_data.time.minutes,
                     packet.gps_data.time.seconds, packet.status);

  if (!check_write(written, remaining_len)) return buf;
  offset += written;
  remaining_len -= written;

  written = snprintf(buf + offset, remaining_len, "%.4f,%.4f,%.4f,",
                     packet.bmp_data.temperature, packet.bmp_data.pressure,
                     packet.bmp_data.altitude);

  if (!check_write(written, remaining_len)) return buf;
  offset += written;
  remaining_len -= written;

  written = snprintf(buf + offset, remaining_len, "%d,%d,%d,%d,%d,%d,",
                     packet.mpu_data.acc_max.x, packet.mpu_data.acc_max.y,
                     packet.mpu_data.acc_max.z, packet.mpu_data.acc_avg.x,
                     packet.mpu_data.acc_avg.y, packet.mpu_data.acc_avg.z);

  if (!check_write(written, remaining_len)) return buf;
  offset += written;
  remaining_len -= written;

  written = snprintf(buf + offset, remaining_len, "%d,%d,%d,%d,%d,%d,",
                     packet.mpu_data.gyro_max.x, packet.mpu_data.gyro_max.y,
                     packet.mpu_data.gyro_max.z, packet.mpu_data.gyro_avg.x,
                     packet.mpu_data.gyro_avg.y, packet.mpu_data.gyro_avg.z);

  if (!check_write(written, remaining_len)) return buf;
  offset += written;
  remaining_len -= written;

  written = snprintf(buf + offset, remaining_len, "%.4f,%.4f,%.4f,%.4f,",
                     packet.mpu_data.rot_avg.x, packet.mpu_data.rot_avg.y,
                     packet.mpu_data.rot_avg.z, packet.mpu_data.rot_avg.w);

  if (!check_write(written, remaining_len)) return buf;
  offset += written;
  remaining_len -= written;

  written = snprintf(buf + offset, remaining_len, "%.4f,%.4f",
                     packet.gps_data.lat, packet.gps_data.lng);

  if (!check_write(written, remaining_len)) return buf;

  return buf;
}

}  // namespace logger
