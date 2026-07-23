#include <iostream>
#include <vector>
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
    // 1. Init Physics Engine
    PolytropicEoS eos(0.025, 2.0);
    TOVSystem tov(eos);

    // 2. Init Graphics Engine
    StarRenderer renderer(1280, 720, "Neutron Star Simulation (Modular)");

    double density_multiplier = 2.0; 
    auto [tov_radius, mass_solar] = calculate_star(Constants::rho_nuc_si * density_multiplier, tov, eos);

    // 3. Main Loop
    while (!renderer.should_close()) {
        
        // Handle logical input
        bool physics_changed = false;
        if (IsKeyDown(KEY_UP)) { density_multiplier += 0.05; physics_changed = true; }
        if (IsKeyDown(KEY_DOWN)) { density_multiplier -= 0.05; physics_changed = true; }
        if (density_multiplier < 0.1) density_multiplier = 0.1;

        // Update Physics
        if (physics_changed) {
            auto result = calculate_star(Constants::rho_nuc_si * density_multiplier, tov, eos);
            tov_radius = result.first;
            mass_solar = result.second;
        }

        // Determine State
        bool is_collapsed = BlackHole::is_collapsed(mass_solar);
        double display_radius = is_collapsed ? BlackHole::get_schwarzschild_radius(mass_solar) : tov_radius;

        // Update Graphics
        renderer.update_input();
        renderer.render_frame(density_multiplier, display_radius, mass_solar, is_collapsed);
    }

    return 0;
}