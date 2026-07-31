#pragma once

#include "raylib.h"

class StarPhysics {
private:
    float spin_angle;
    float accretion_angle;
    float spin_rate;
    float precession_angle;
    float precession_rate;
    float nutation_angle;
    bool paused;

public:
    StarPhysics();

    void update(float dt);
    void toggle_pause();
    bool is_paused() const;
    
    float get_spin_angle() const;
    float get_accretion_angle() const;
    Vector3 get_rotation_axis() const;

    void update_hydrostatic_equilibrium(double &density, double &radius, double &mass);
};