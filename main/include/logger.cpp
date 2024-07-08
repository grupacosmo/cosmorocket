#include "logger.h"

namespace log {
namespace {
int serialize_data(char *buf, size_t len, const Packet &packet) {
    return snprintf(
        buf, len,
        "%u,%u,%u,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%"
        "0."
        "4f,%0.4f",
        packet.n, packet.sys_time, packet.status, packet.bmp_data.temperature,
        packet.bmp_data.pressure, packet.bmp_data.altitude,
        packet.mpu_data.max_x, packet.mpu_data.max_y, packet.mpu_data.max_z,
        packet.mpu_data.avg_x, packet.mpu_data.avg_y, packet.mpu_data.avg_z,
        packet.mpu_data.rot_x, packet.mpu_data.rot_y, packet.mpu_data.rot_z);
}
} // namespace

String serialize(Packet &packet) {
    char buf[256];
    if (serialize_data(&buf[0], 256, packet)) {
        Serial.println("Failed to serialize data, check buffer size");
        return String("0,0,0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0."
                      "0,0.0");
    }

    return String(buf);
}
} // namespace log