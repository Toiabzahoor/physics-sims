#pragma once
#include "raylib.h"
#include <vector>

namespace BlackHole {
    constexpr double TOV_LIMIT_SOLAR = 2.1;
    bool is_collapsed(double mass_solar);
    double get_schwarzschild_radius(double mass_solar);

    struct DiskParticle {
        float distance;    
        float angle;       // Current orbital angle
        float y_offset;    // Vertical variance to give the disk 3D volume
        float base_speed;  // Keplerian orbital speed
        float size;        // Render size
    };

    class Visuals {
    private:
        std::vector<DiskParticle> particles;
        bool is_initialized;

    public:
        Visuals();
        ~Visuals();
        
        void init(); 
        void cleanup();
        void update(float dt, float horizon_radius);
        void draw(float horizon_radius, Vector3 camera_pos);
    };
}