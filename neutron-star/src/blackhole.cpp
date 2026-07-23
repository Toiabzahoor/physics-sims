#include "../include/blackhole.hpp"

namespace BlackHole {
    bool is_collapsed(double mass_solar) {
        return mass_solar > TOV_LIMIT_SOLAR;
    }

    double get_schwarzschild_radius(double mass_solar) {
        return 2.95 * mass_solar;
    }
}