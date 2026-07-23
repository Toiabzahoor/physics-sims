#pragma once

namespace BlackHole {
    constexpr double TOV_LIMIT_SOLAR = 2.1;

    bool is_collapsed(double mass_solar);
    double get_schwarzschild_radius(double mass_solar);
}