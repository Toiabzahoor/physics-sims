#include <iostream>
#include <vector>
#include <tuple>
#include "raylib.h"

#include "../include/constants.hpp"
#include "../include/eos.hpp"
#include "../include/tov.hpp"
#include "../include/rk4.hpp"
#include "../include/blackhole.hpp"
#include "../include/renderer.hpp"
#include "../include/star_physics.hpp"

std::pair<double, double> solve_tov(double central_density) {
    PolytropicEoS eos(0.025, 2.0); 
    TOVSystem tov(eos);

    double P_c = eos.returnPressure(central_density);
    double r = 10.0; 
    std::vector<double> y = { P_c, 0.0 }; 
    double dr = 10.0;

    auto derivs = [&tov](double r, const std::vector<double>& state) {
        return tov.getDerivatives(r, state);
    };

    while (y[0] > 0.0 && r < 50000.0) { 
        y = RK4::step(r, y, dr, derivs);
        r += dr;
    }

    double R_km = r / 1000.0; 
    double M_solar = y[1] / Constants::M_sun; 
    
    return {R_km, M_solar};
}

int main() {
    StarRenderer renderer(1280, 720, "Neutron Star TOV Simulation");
    
    StarPhysics physics;

    double density_multiplier = 1.0;

    while (!renderer.should_close()) {
        
        if (IsKeyDown(KEY_W)) density_multiplier += 0.02;
        if (IsKeyDown(KEY_S)) density_multiplier -= 0.02;
        if (density_multiplier < 0.1) density_multiplier = 0.1;

        double central_density = density_multiplier * Constants::rho_nuc_si;
        
        auto result = solve_tov(central_density);
        double display_radius = result.first;
        double mass_solar = result.second;
        
        bool is_black_hole = BlackHole::is_collapsed(mass_solar);

        physics.update(GetFrameTime());

        renderer.update_input(physics);

        renderer.render_frame(density_multiplier, display_radius, mass_solar, is_black_hole, physics);
    }

    return 0;
}