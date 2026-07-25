#include "../include/renderer.hpp"
#include "../include/blackhole.hpp"
#include "rlgl.h"
#include "raymath.h"
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
    
    cameraDistance = 150.0f;
    cameraAngleX = 0.785f;
    cameraAngleY = 0.35f;
    animated_radius = 0.0f;

    Image noise = GenImagePerlinNoise(512, 512, 0, 0, 8.0f);
    ImageColorTint(&noise, (Color){ 220, 220, 255, 255 });
    starTexture = LoadTextureFromImage(noise);
    UnloadImage(noise);

    Mesh sphereMesh = GenMeshSphere(1.0f, 64, 64);
    starModel = LoadModelFromMesh(sphereMesh);
    starModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = starTexture;

    bhShader = LoadShaderFromMemory(nullptr, BlackHole::bh_shader_code);
    resLoc = GetShaderLocation(bhShader, "resolution");
    camPosLoc = GetShaderLocation(bhShader, "camPos");
    camDirLoc = GetShaderLocation(bhShader, "camDir");
    camUpLoc = GetShaderLocation(bhShader, "camUp");
    camRightLoc = GetShaderLocation(bhShader, "camRight");
    rsLoc = GetShaderLocation(bhShader, "rs");
    timeLoc = GetShaderLocation(bhShader, "time");

    jets.init();
    SetTargetFPS(60);
}

StarRenderer::~StarRenderer() {
    jets.cleanup();
    UnloadShader(bhShader);
    UnloadTexture(starTexture);
    UnloadModel(starModel);
    CloseWindow();
}

bool StarRenderer::should_close() const {
    return WindowShouldClose();
}

void StarRenderer::update_input(StarPhysics& physics) {
    if (IsKeyPressed(KEY_SPACE)) physics.toggle_pause();

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        cameraDistance -= wheel * (cameraDistance * 0.1f);
        if (cameraDistance < 10.0f) cameraDistance = 10.0f;
        if (cameraDistance > 900.0f) cameraDistance = 900.0f;
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

    // Calculate the camera position using spherical coordinates
    camera.position.x = cameraDistance * cosf(cameraAngleY) * cosf(cameraAngleX);
    camera.position.y = cameraDistance * sinf(cameraAngleY);
    camera.position.z = cameraDistance * cosf(cameraAngleY) * sinf(cameraAngleX);

    // Dynamically calculate the 'up' vector to prevent flipping at the poles
    // This completely removes the need for clamping the viewing angle.
    camera.up.x = -sinf(cameraAngleY) * cosf(cameraAngleX);
    camera.up.y = cosf(cameraAngleY);
    camera.up.z = -sinf(cameraAngleY) * sinf(cameraAngleX);
}

void StarRenderer::draw_neutron_star(double mass, const StarPhysics& physics) {
    Color coreColor = RAYWHITE;
    Color glowColor = SKYBLUE;
    
    if (mass > 1.5) { coreColor = YELLOW; glowColor = ORANGE; }
    if (mass > 1.9) { coreColor = ORANGE; glowColor = RED; }
    
    Vector3 scale = { animated_radius, animated_radius, animated_radius };
    Vector3 axis = physics.get_rotation_axis();

    float angle = acosf(Vector3DotProduct({0.0f, 1.0f, 0.0f}, axis)) * RAD2DEG;
    Vector3 cross = Vector3CrossProduct({0.0f, 1.0f, 0.0f}, axis);
    Vector3 rotationAxis = Vector3Length(cross) > 0.001f ? Vector3Normalize(cross) : (Vector3){1.0f, 0.0f, 0.0f};

    rlPushMatrix();
    rlRotatef(angle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
    rlRotatef(physics.get_spin_angle(), 0.0f, 1.0f, 0.0f);
    
    DrawModelEx(starModel, (Vector3){0,0,0}, (Vector3){0,1,0}, 0.0f, scale, coreColor);
    rlPopMatrix();

    float time = (float)GetTime();
    float pulse1 = 1.02f + 0.01f * sinf(time * 5.0f);
    float pulse2 = 1.08f + 0.02f * sinf(time * 3.0f + 1.0f);
    float pulse3 = 1.15f + 0.03f * sinf(time * 2.0f + 2.0f);

    BeginBlendMode(BLEND_ADDITIVE);
    DrawSphereEx((Vector3){0,0,0}, animated_radius * pulse1, 64, 64, Fade(glowColor, 0.4f));
    DrawSphereEx((Vector3){0,0,0}, animated_radius * pulse2, 64, 64, Fade(glowColor, 0.2f));
    DrawSphereEx((Vector3){0,0,0}, animated_radius * pulse3, 64, 64, Fade(glowColor, 0.1f));
    EndBlendMode();
}

void StarRenderer::draw_shader_background(float rs) {
    Vector3 camPos = camera.position;
    Vector3 camDir = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(camDir, camera.up));
    Vector3 camUp = Vector3CrossProduct(camRight, camDir);
    
    float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
    float time = (float)GetTime();

    SetShaderValue(bhShader, resLoc, res, SHADER_UNIFORM_VEC2);
    SetShaderValue(bhShader, camPosLoc, &camPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(bhShader, camDirLoc, &camDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(bhShader, camUpLoc, &camUp, SHADER_UNIFORM_VEC3);
    SetShaderValue(bhShader, camRightLoc, &camRight, SHADER_UNIFORM_VEC3);
    SetShaderValue(bhShader, rsLoc, &rs, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bhShader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(bhShader);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
    EndShaderMode();
}

void StarRenderer::draw_ui(double density, double radius, double mass, bool is_black_hole, const StarPhysics& physics) {
    DrawRectangle(10, 10, 480, 150, Fade(BLACK, 0.8f));
    DrawText("Controls: Arrow Keys to Rotate | Scroll to Zoom | SPACE to Pause", 20, 20, 10, LIGHTGRAY);
    DrawText("Physics: W / S keys for central density", 20, 35, 10, LIGHTGRAY);
    
    DrawText(TextFormat("Central Density: %.2fx Nuclear", density), 20, 60, 20, WHITE);
    
    if (is_black_hole) {
        DrawText(TextFormat("Schwarzschild Radius: %.2f km", radius), 20, 90, 20, GRAY);
        DrawText(TextFormat("Mass: %.2f M_sun", mass), 20, 120, 20, DARKGRAY);
    } else {
        DrawText(TextFormat("Star Radius: %.2f km", radius), 20, 90, 20, SKYBLUE);
        DrawText(TextFormat("Star Mass: %.2f M_sun", mass), 20, 120, 20, ORANGE);
    }

    if (physics.is_paused()) DrawText("PAUSED", 20, 170, 30, RED);
}

void StarRenderer::render_frame(double density_multiplier, double target_radius, double mass_solar, bool is_black_hole, const StarPhysics& physics) {
    static BlackHole::Visuals bh_visuals;
    static float animated_rs = 0.0f;
    static float disk_alpha = 0.0f;
    
    if (animated_radius == 0.0f && !is_black_hole) animated_radius = target_radius;

    double core_mass = mass_solar * 0.80;
    float target_rs = (float)BlackHole::get_schwarzschild_radius(core_mass);

    if (!physics.is_paused()) {
        if (is_black_hole) {
            bh_visuals.init();
            bh_visuals.update(GetFrameTime(), animated_rs);
            animated_radius += (0.0f - animated_radius) * 0.1f;
            animated_rs += (target_rs - animated_rs) * 0.05f;
            disk_alpha += (1.0f - disk_alpha) * 0.02f;
        } else {
            animated_radius += (target_radius - animated_radius) * 0.2f;
            animated_rs += (0.0f - animated_rs) * 0.1f;
            disk_alpha += (0.0f - disk_alpha) * 0.1f;
            
            float escape_v = 80.0f * (float)(mass_solar / target_radius);
            jets.update(GetFrameTime(), is_black_hole, physics.get_rotation_axis(), escape_v);
        }
    }

    BeginDrawing();
    ClearBackground(BLACK);
    draw_shader_background(animated_rs);

    BeginMode3D(camera);
    
    if (animated_radius > 0.1f && !is_black_hole) {
        draw_neutron_star(mass_solar, physics);
        jets.draw(camera, 1.0f - disk_alpha);
    }
    
    if (disk_alpha > 0.01f) {
        bh_visuals.draw(animated_rs, camera, disk_alpha);
    }
    
    EndMode3D();
    draw_ui(density_multiplier, target_radius, mass_solar, is_black_hole, physics);
    EndDrawing();
}