#pragma once
#include <algorithm>
class Pressure {
  private:
    static constexpr unsigned long max_pa = 6000000;
    static constexpr unsigned long pa_per_atm = 101325;
    static constexpr float max_current = 20.f;

    static float min_current;
    static float pa_per_ma;

    static auto pa_from_ma(float current) -> float {
        return std::max(0.f, pa_per_ma * (current - min_current));
    }

    float pascals;

    Pressure(float pascals = 0.f) : pascals{pascals} {}

  public:
    static auto set_min_current(float new_min) -> void {
        min_current = new_min;
        pa_per_ma = max_pa / (max_current - min_current);
    }

    static auto get_min_current() -> float { return min_current; }

    static auto relative(float current) -> Pressure {
        return Pressure(pa_from_ma(current));
    }

    static auto absolute(float current) -> Pressure {
        return Pressure(pa_from_ma(current) + 101325);
    }

    static auto empty() -> Pressure { return Pressure(); }

    /// returns presssure in Pa above 1atm
    auto Pa() const -> float { return pascals; }

    /// returns presssure in hPa above 1atm
    auto hPa() const -> float { return pascals / 100; }

    /// returns presssure in kPa above 1atm
    auto kPa() const -> float { return pascals / 1000; }

    /// returns presssure in mPa above 1atm
    auto mPa() const -> float { return pascals / 1000000; }

    /// returns pressure in atm above 1atm
    auto atm() const -> float { return pascals / 101325; }

    /// returns presssure in bars above 1atm
    auto bar() const -> float { return pascals / 100000; }

    auto psi() const -> float { return pascals / 6894.7572932; }
};
