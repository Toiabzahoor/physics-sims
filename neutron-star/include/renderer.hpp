#pragma once
#include "raylib.h"
#include <vector>
#include "star_physics.hpp"

struct StarData {
    Vector3 position;
    float brightness;
};

class StarRenderer {
private:
    Camera3D camera;
    std::vector<StarData> background_stars; 
    float cameraDistance;
    float cameraAngleX;
    float cameraAngleY;
    
    float animated_radius;

    Model starModel;       
    Texture2D starTexture; 

    Shader bhShader;
    int resLoc, camPosLoc, camDirLoc, camUpLoc, camRightLoc, rsLoc, timeLoc;

    void draw_background();
    void draw_stars_3d();
    void draw_neutron_star(double mass, const StarPhysics& physics);
    void draw_black_hole(float rs);
    void draw_ui(double density, double radius, double mass, bool is_black_hole, const StarPhysics& physics);

public:
    StarRenderer(int width, int height, const char* title);
    ~StarRenderer();
    void update_input(StarPhysics& physics);
    void render_frame(double density_multiplier, double target_radius, double mass_solar, bool is_black_hole, const StarPhysics& physics);
    bool should_close() const;
};