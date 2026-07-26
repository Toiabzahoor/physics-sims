#pragma once

#include "raylib.h"
#include <vector>

struct JetParticle {
    Vector3 position;
    Vector3 velocity;
    float life;
    float max_life;
    float size;
    Color color;
};

class PolarJets {
private:
    std::vector<JetParticle> particles;
    Texture2D jetTex;
    bool is_initialized;

public:
    PolarJets();
    ~PolarJets();

    void init();
    void cleanup();
    void update(float dt, bool is_black_hole, Vector3 axis, float escape_velocity);
    void draw(Camera3D camera, float alpha_multiplier);
};