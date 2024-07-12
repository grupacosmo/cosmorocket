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
void init();
void get_mpu(void *pvParameters);
} // namespace mpu