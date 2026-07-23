#include "../include/renderer.hpp"
#include <string>
#include <cmath>

StarRenderer::StarRenderer(int width, int height, const char* title) {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, title);

    camera = { 0 };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    cameraDistance = 40.0f;

    for(int i = 0; i < 300; i++) {
        background_stars.push_back({(float)GetRandomValue(0, width), (float)GetRandomValue(0, height)});
    }
    SetTargetFPS(60);
}

StarRenderer::~StarRenderer() {
    CloseWindow();
}

bool StarRenderer::should_close() const {
    return WindowShouldClose();
}

void StarRenderer::update_input() {
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        cameraDistance -= wheel * 3.0f;
        if (cameraDistance < 10.0f) cameraDistance = 10.0f;
        if (cameraDistance > 100.0f) cameraDistance = 100.0f;
    }

    float angle = GetTime() * 0.2f; 
    camera.position.x = cameraDistance * cos(angle);
    camera.position.z = cameraDistance * sin(angle);
    camera.position.y = 15.0f;
}

void StarRenderer::draw_background() {
    ClearBackground((Color){ 2, 2, 8, 255 }); 
    for(auto& star : background_stars) {
        DrawPixelV(star, Fade(WHITE, (float)GetRandomValue(20, 80) / 100.0f));
    }
}

void StarRenderer::draw_star(float radius, double mass, bool is_black_hole) {
    Color coreColor = RAYWHITE;
    Color glowColor = SKYBLUE;
    
    if (!is_black_hole) {
        if (mass > 1.5) { coreColor = YELLOW; glowColor = ORANGE; }
        if (mass > 1.9) { coreColor = ORANGE; glowColor = RED; }
    } else {
        coreColor = BLACK; 
    }

    if (!is_black_hole) {
        DrawSphereEx((Vector3){0,0,0}, radius * 1.3f, 64, 64, Fade(glowColor, 0.05f));
        DrawSphereEx((Vector3){0,0,0}, radius * 1.15f, 64, 64, Fade(glowColor, 0.15f));
        DrawSphereEx((Vector3){0,0,0}, radius * 1.05f, 64, 64, Fade(glowColor, 0.3f));
    }
    
    DrawSphereEx((Vector3){0,0,0}, radius, 64, 64, coreColor);
    
    if (is_black_hole) {
        DrawCircle3D((Vector3){0,0,0}, radius * 1.5f, (Vector3){1,0,0}, 90.0f, Fade(ORANGE, 0.4f));
    }
}

void StarRenderer::draw_ui(double density, double radius, double mass, bool is_black_hole) {
    DrawRectangle(10, 10, 480, 160, Fade(BLACK, 0.8f));
    DrawText("Controls: UP/DOWN for density | Mouse Wheel to Zoom", 20, 20, 10, LIGHTGRAY);
    DrawText(TextFormat("Central Density: %.2fx Nuclear", density), 20, 50, 20, WHITE);
    
    if (is_black_hole) {
        DrawText("EVENT HORIZON FORMED", 20, 80, 20, RED);
        DrawText(TextFormat("Schwarzschild Radius: %.2f km", radius), 20, 110, 20, GRAY);
        DrawText(TextFormat("Mass: %.2f M_sun", mass), 20, 140, 20, DARKGRAY);
    } else {
        DrawText(TextFormat("Star Radius: %.2f km", radius), 20, 80, 20, SKYBLUE);
        DrawText(TextFormat("Star Mass: %.2f M_sun", mass), 20, 110, 20, ORANGE);
    }
}

void StarRenderer::render_frame(double density_multiplier, double display_radius, double mass_solar, bool is_black_hole) {
    BeginDrawing();
    draw_background();
    
    BeginMode3D(camera);
    draw_star((float)display_radius, mass_solar, is_black_hole);
    EndMode3D();

    draw_ui(density_multiplier, display_radius, mass_solar, is_black_hole);
    EndDrawing();
}