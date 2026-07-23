#pragma once
#include "raylib.h"
#include <vector>

namespace BlackHole {
    constexpr double TOV_LIMIT_SOLAR = 2.1;
    
    bool is_collapsed(double mass_solar);
    double get_schwarzschild_radius(double mass_solar);

    extern const char* bh_shader_code;

    struct DiskParticle {
        float distance;
        float angle;       
        float y_offset;    
        float base_speed;  
        float size;        
    };

    class Visuals {
    private:
        std::vector<DiskParticle> particles;
        Texture2D particleTex;
        bool is_initialized;

    public:
        Visuals();
        ~Visuals();
        
        void init(); 
        void cleanup();
        void update(float dt, float horizon_radius);
        void draw(float horizon_radius, Camera3D camera, float alpha_multiplier);
    };
}