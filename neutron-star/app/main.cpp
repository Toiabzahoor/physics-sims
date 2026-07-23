#include <iostream>
#include <vector>
#include <cmath>
#include "raylib.h"
#include "../include/constants.hpp"
#include "../include/eos.hpp"
#include "../include/tov.hpp"
#include "../include/rk4.hpp"
#include "../include/blackhole.hpp"
#include "../include/renderer.hpp"

std::pair<double, double> calculate_star(double central_density, const TOVSystem& tov, const EquationOfState& eos) {
    double central_pressure = eos.returnPressure(central_density);
    
    std::vector<double> y = {central_pressure, 0.0}; 
    double r = 1.0; 
    double dr = 10.0;
    
    RK4::DerivativeFunc derivs = [&tov](double r, const std::vector<double>& y) {
        return tov.getDerivatives(r, y);
    };
    
    while (y[0] > 0.0) {
        y = RK4::step(r, y, dr, derivs);
        r += dr;
        if (r > 100000.0) break;
    }
    return {r / Constants::km_to_m, y[1] / Constants::M_sun};
}

int main() {
    // Reverted to the stable physics that actually produce a visible star
    PolytropicEoS eos(0.025, 2.0);
    TOVSystem tov(eos);

    StarRenderer renderer(1600, 900, "Neutron Star Simulation");

    double density_multiplier = 1.5; 
    auto [tov_radius, mass_solar] = calculate_star(Constants::rho_nuc_si * density_multiplier, tov, eos);

    double critical_density_multiplier = 0.0;

    while (!renderer.should_close()) {
        
        bool physics_changed = false;
        
        if (IsKeyDown(KEY_W)) { density_multiplier += 0.05; physics_changed = true; }
        if (IsKeyDown(KEY_S)) { density_multiplier -= 0.05; physics_changed = true; }
        if (density_multiplier < 0.1) density_multiplier = 0.1;

        if (physics_changed) {
            auto result = calculate_star(Constants::rho_nuc_si * density_multiplier, tov, eos);
            
            if (!std::isnan(result.first) && !std::isnan(result.second)) {
                tov_radius = result.first;
                mass_solar = result.second;
            }

            // The TOV limit for this specific EoS is ~2.1
            if (mass_solar > 2.1 && critical_density_multiplier == 0.0) {
                critical_density_multiplier = density_multiplier;
            }
        }

        bool is_black_hole = (critical_density_multiplier > 0.0 && density_multiplier >= critical_density_multiplier);
        double display_radius = is_black_hole ? BlackHole::get_schwarzschild_radius(mass_solar) : tov_radius;

        renderer.update_input();
        renderer.render_frame(density_multiplier, display_radius, mass_solar, is_black_hole);
    }

    return 0;
}