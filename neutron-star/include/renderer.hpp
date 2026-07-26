#pragma once

#include "raylib.h"
#include "star_physics.hpp"
#include "jets.hpp"
#include "blackhole.hpp"

class StarRenderer {
private:
    Camera3D camera;
    float cameraDistance;
    float cameraAngleX;
    float cameraAngleY;
    float animated_radius;

    Model starModel;
    Texture2D starTexture;

    Shader bhShader;
    int resLoc, camPosLoc, camDirLoc, camUpLoc, camRightLoc, rsLoc, timeLoc;

    Shader nsShader;
    int viewPosLoc, poleAxisLoc, massLoc, radiusLoc, nsTimeLoc;

    PolarJets jets;

    BlackHole::Visuals bh_visuals;
    float animated_rs;
    float disk_alpha;

    void draw_neutron_star(double mass, const StarPhysics& physics);
    void draw_shader_background(float rs);
    void draw_ui(double density, double radius, double mass, bool is_black_hole, const StarPhysics& physics);

public:
    StarRenderer(int width, int height, const char* title);
    ~StarRenderer();

    void update_input(StarPhysics& physics);
    void render_frame(double density_multiplier, double target_radius, double mass_solar, bool is_black_hole, const StarPhysics& physics);
    bool should_close() const;
};