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
    cameraAngleX = 0.785f; 
    cameraAngleY = 0.35f;  

    animated_radius = 0.0f;
    accretion_rotation = 0.0f;

    for(int i = 0; i < 600; i++) {
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
        // Proportional zoom: Faster when far away, slower when close
        cameraDistance -= wheel * (cameraDistance * 0.1f);
        
        // Massive bounds: Allow zooming from 2km away to 1000km away
        if (cameraDistance < 2.0f) cameraDistance = 2.0f;
        if (cameraDistance > 1000.0f) cameraDistance = 1000.0f;
    }

    float rotationSpeed = 0.03f;
    if (IsKeyDown(KEY_LEFT)) cameraAngleX += rotationSpeed;
    if (IsKeyDown(KEY_RIGHT)) cameraAngleX -= rotationSpeed;
    if (IsKeyDown(KEY_UP)) cameraAngleY -= rotationSpeed;
    if (IsKeyDown(KEY_DOWN)) cameraAngleY += rotationSpeed;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 delta = GetMouseDelta();
        cameraAngleX -= delta.x * 0.005f;
        cameraAngleY -= delta.y * 0.005f;
    }

    if (cameraAngleY > 1.5f) cameraAngleY = 1.5f;
    if (cameraAngleY < -1.5f) cameraAngleY = -1.5f;

    camera.position.x = cameraDistance * cos(cameraAngleY) * cos(cameraAngleX);
    camera.position.z = cameraDistance * cos(cameraAngleY) * sin(cameraAngleX);
    camera.position.y = cameraDistance * sin(cameraAngleY);
}

void StarRenderer::draw_background() {
    ClearBackground((Color){ 2, 2, 8, 255 }); 
    for(auto& star : background_stars) {
        DrawPixelV(star, Fade(WHITE, (float)GetRandomValue(20, 80) / 100.0f));
    }
}

void StarRenderer::draw_star(float target_radius, double mass, bool is_black_hole) {
    if (animated_radius == 0.0f) animated_radius = target_radius;
    
    float lerp_speed = is_black_hole ? 0.1f : 0.2f;
    animated_radius += (target_radius - animated_radius) * lerp_speed;

    Color coreColor = RAYWHITE;
    Color glowColor = SKYBLUE;
    
    if (!is_black_hole) {
        if (mass > 1.5) { coreColor = YELLOW; glowColor = ORANGE; }
        if (mass > 1.9) { coreColor = ORANGE; glowColor = RED; }
    } else {
        coreColor = BLACK; 
    }

    DrawSphereEx((Vector3){0,0,0}, animated_radius, 64, 64, coreColor);

    if (!is_black_hole) {
        DrawSphereEx((Vector3){0,0,0}, animated_radius * 1.05f, 64, 64, Fade(glowColor, 0.3f));
        DrawSphereEx((Vector3){0,0,0}, animated_radius * 1.15f, 64, 64, Fade(glowColor, 0.15f));
        DrawSphereEx((Vector3){0,0,0}, animated_radius * 1.3f, 64, 64, Fade(glowColor, 0.05f));
    } else {
        accretion_rotation += 1.5f; 
        Vector3 tiltAxis = {0.2f, 1.0f, 0.2f}; 
        
        for(float r = 1.2f; r < 3.0f; r += 0.05f) {
            float alpha = (3.0f - r) * 0.4f; 
            DrawCircle3D((Vector3){0,0,0}, animated_radius * r, tiltAxis, accretion_rotation, Fade(ORANGE, alpha));
            DrawCircle3D((Vector3){0,0,0}, animated_radius * r, tiltAxis, accretion_rotation, Fade(YELLOW, alpha * 0.5f));
        }
    }
}

void StarRenderer::draw_ui(double density, double radius, double mass, bool is_black_hole) {
    DrawRectangle(10, 10, 480, 150, Fade(BLACK, 0.8f));
    DrawText("Controls: Arrow Keys to Rotate | Scroll to Zoom", 20, 20, 10, LIGHTGRAY);
    DrawText("Physics: W / S keys for central density", 20, 35, 10, LIGHTGRAY);
    
    DrawText(TextFormat("Central Density: %.2fx Nuclear", density), 20, 60, 20, WHITE);
    
    if (is_black_hole) {
        DrawText(TextFormat("Schwarzschild Radius: %.2f km", radius), 20, 90, 20, GRAY);
        DrawText(TextFormat("Mass: %.2f M_sun", mass), 20, 120, 20, DARKGRAY);
    } else {
        DrawText(TextFormat("Star Radius: %.2f km", radius), 20, 90, 20, SKYBLUE);
        DrawText(TextFormat("Star Mass: %.2f M_sun", mass), 20, 120, 20, ORANGE);
    }
}

void StarRenderer::render_frame(double density_multiplier, double target_radius, double mass_solar, bool is_black_hole) {
    BeginDrawing();
    draw_background();
    
    BeginMode3D(camera);
    draw_star((float)target_radius, mass_solar, is_black_hole);
    EndMode3D();

    draw_ui(density_multiplier, target_radius, mass_solar, is_black_hole);
    EndDrawing();
}