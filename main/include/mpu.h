#pragma once

namespace mpu {

template<typename T>
struct Position {
    T x{};
    T y{};
    T z{};
};

struct Data {
    Position<float> max{};
    Position<float> avg{};
    Position<float> rot{};
};

void init();
bool calibrate();
void mpu_task(void *pvParameters);
void mpu_print(void *pvParameters);
void print_data();
Data get_data();

} // namespace mpu
