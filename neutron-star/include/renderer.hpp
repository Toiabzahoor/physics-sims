#pragma once
#include "raylib.h"
#include <vector>

class StarRenderer {
private:
    Camera3D camera;
    std::vector<Vector2> background_stars;
    float cameraDistance;

    void draw_background();
    void draw_star(float radius, double mass, bool is_black_hole);
    void draw_ui(double density, double radius, double mass, bool is_black_hole);

public:
    StarRenderer(int width, int height, const char* title);
    ~StarRenderer();

    void update_input();
    void render_frame(double density_multiplier, double display_radius, double mass_solar, bool is_black_hole);
    bool should_close() const;
};