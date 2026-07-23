#include "../include/star_physics.hpp"

StarPhysics::StarPhysics() {
    spin_angle = 0.0f;
    accretion_angle = 0.0f;
    spin_rate = 800.0f; // degrees per second
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

void StarPhysics::update_hydrostatic_equilibrium(double &density, double &radius) {
}