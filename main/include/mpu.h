#pragma once

namespace mpu {

struct Position {
  float x{};
  float y{};
  float z{};
};

struct Data {
  Position max{};
  Position avg{};
  Position rot{};
};

void init();
bool calibrate();
void mpu_task(void *pvParameters);
void mpu_print(void *pvParameters);
void pretty_print();
Data get_mpu();

} // namespace mpu