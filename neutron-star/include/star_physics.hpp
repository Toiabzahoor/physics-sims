#pragma once

class StarPhysics {
private:
    float spin_angle;
    float accretion_angle;
    float spin_rate;
    bool paused;

public:
    StarPhysics();

    void update(float dt);
    
    void toggle_pause();
    bool is_paused() const;
    
    float get_spin_angle() const;
    float get_accretion_angle() const;

    void update_hydrostatic_equilibrium(double &density, double &radius);
};