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

} // namespace mpu
