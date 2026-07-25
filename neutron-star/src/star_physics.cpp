#include "../include/star_physics.hpp"
#include "../include/tov.hpp"
#include "../include/rk4.hpp"
#include "../include/constants.hpp"
#include <math.h>
#include <iostream>

StarPhysics::StarPhysics() {
    spin_angle = 0.0f;
    accretion_angle = 0.0f;
    spin_rate = 800.0f; // degrees per second
    precession_angle = 0.0f;
    precession_rate = 45.0f;
    nutation_angle = 0.2f;
    paused = false;
}

void StarPhysics::update(float dt) {
    if (!paused) {
        // High-speed neutron star spin
        spin_angle += spin_rate * dt;
        if (spin_angle >= 360.0f) spin_angle -= 360.0f;
        
        // The accretion disk spins slower than the star itself
        accretion_angle += (spin_rate * 0.15f) * dt;
        if (accretion_angle >= 360.0f) accretion_angle -= 360.0f;

        precession_angle += precession_rate * dt;
        if (precession_angle >= 360.0f) precession_angle -= 360.0f;
    }
}

void StarPhysics::toggle_pause() {
    paused = !paused;
}

bool StarPhysics::is_paused() const {
    return paused;
}

float StarPhysics::get_spin_angle() const {
    return spin_angle;
}

float StarPhysics::get_accretion_angle() const {
    return accretion_angle;
}

Vector3 StarPhysics::get_rotation_axis() const {
    float p_rad = precession_angle * DEG2RAD;
    return {
        sinf(nutation_angle) * cosf(p_rad),
        cosf(nutation_angle),
        sinf(nutation_angle) * sinf(p_rad)
    };
}

void StarPhysics::update_hydrostatic_equilibrium(double &density, double &radius) {
    PolytropicEoS eos(100.0, 2.0); 
    TOVSystem tov(eos);

    double P_c = eos.returnPressure(density * Constants::rho_nuc_si);
    double epsilon_c = eos.returnEnergyDensity(P_c);
    
    double dr = 10.0;
    double r = dr;
    
    double c2 = std::pow(Constants::c, 2);
    double c4 = std::pow(Constants::c, 4);
    double G = Constants::G;

    double M_initial = (4.0 / 3.0) * M_PI * std::pow(r, 3) * (epsilon_c / c2);
    double P_initial = P_c - (2.0 * M_PI * G / (3.0 * c4)) * 
                       (epsilon_c + P_c) * (3.0 * P_c + epsilon_c) * std::pow(r, 2);
    
    std::vector<double> y = {P_initial, M_initial};
    
    auto derivs = [&tov](double current_r, const std::vector<double>& current_y) {
        return tov.getDerivatives(current_r, current_y);
    };

    double P_prev = y[0];
    double r_prev = r;
    
    int max_steps = 100000; 
    int step_count = 0;

    while (y[0] > 0.0 && step_count < max_steps) {
        P_prev = y[0];
        r_prev = r;
        
        y = RK4::step(r, y, dr, derivs);
        r += dr;
        step_count++;
    }

    double exact_radius = r;
    if (y[0] < 0.0) {
        double fraction = P_prev / (P_prev - y[0]);
        exact_radius = r_prev + fraction * dr;
    }

    radius = exact_radius / Constants::km_to_m;
}