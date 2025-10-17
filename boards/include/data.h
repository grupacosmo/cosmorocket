#pragma once

#include <atomic>

struct Data {
    using Pressure = float;
    using Temperature = float;

    std::atomic<Pressure> pressure_;
    std::atomic<Temperature> temperature_;
};
