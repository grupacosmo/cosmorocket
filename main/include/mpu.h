#pragma once
namespace mpu {
struct Data {
    float max_x;
    float max_y;
    float max_z;
    float avg_x;
    float avg_y;
    float avg_z;
    float rot_x;
    float rot_y;
    float rot_z;
};

static unsigned int count = 0;
static Data internal{};
static float gyro_res, accel_res;
static bool init_success = false;

void init();
bool calibrate();
void mpu_task(void *pvParameters);
void mpu_print(void *pvParameters);
Data get_mpu();

} // namespace mpu
