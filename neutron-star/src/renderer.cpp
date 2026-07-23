#include "../include/renderer.hpp"
#include "../include/blackhole.hpp"
#include "raymath.h" 
#include <string>
#include <cmath>

const char* bh_shader_code = R"(
#version 330

out vec4 finalColor;

uniform vec2 resolution;
uniform vec3 camPos;
uniform vec3 camDir;
uniform vec3 camUp;
uniform vec3 camRight;
uniform float rs;
uniform float time;

void main() {
    vec2 uv = (gl_FragCoord.xy / resolution.xy - 0.5) * 2.0;
    uv.x *= resolution.x / resolution.y;
    
    vec3 rayDir = normalize(camDir + camRight * uv.x + camUp * uv.y);
    vec3 p = camPos;
    vec3 v = rayDir;
    
    vec3 col = vec3(0.0);
    float accumulated_density = 0.0;
    
    for (int i = 0; i < 400; i++) {
        float r = length(p);
        
        if (r < rs) {
            break;
        }
        if (r > max(2000.0, length(camPos) * 1.5)) {
            break;
        }
        
        float dt = max(r * 0.02, rs * 0.01); 
        
        vec3 gravity = -p * (1.5 * rs / max(r * r * r, 0.001));
        v = normalize(v + gravity * dt);
        p += v * dt;
        
        if (abs(p.y) < (rs * 0.1 + dt * 0.5) && r > rs * 1.0 && r < rs * 12.0) {
            float isco = rs * 3.0;
            float distNorm = r / isco;
            
            float temp = 1.0 / (distNorm * distNorm);
            if (r < isco) temp *= max((r - rs) / (isco - rs), 0.0); 
            
            vec3 orbit_vel = normalize(vec3(-p.z, 0.0, p.x));
            float doppler = 1.0 + dot(orbit_vel, v) * 0.8;
            
            vec3 emission = vec3(1.0, 0.3, 0.05) * temp * doppler;
            float heat_shift = clamp((temp - 0.8) * 1.5, 0.0, 1.0);
            emission = mix(emission, vec3(0.8, 0.9, 1.0), heat_shift); 
            
            col += emission * dt * 0.2;
            accumulated_density += 0.1 * dt;
            
            if (accumulated_density >= 1.0) break;
        }
    }
    
    if (accumulated_density < 1.0) {
        float star = fract(sin(dot(v.xy, vec2(12.9898, 78.233))) * 43758.5453);
        if (star > 0.995) col += vec3(1.0) * (1.0 - accumulated_density);
    }
    
    col = vec3(1.0) - exp(-col * 1.5);
    
    finalColor = vec4(col, 1.0);
}
)";

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
    
    for(int i = 0; i < 2000; i++) {
        float u = (float)GetRandomValue(0, 1000) / 1000.0f;
        float v = (float)GetRandomValue(0, 1000) / 1000.0f;
        float theta = 2.0f * PI * u;
        float phi = acosf(2.0f * v - 1.0f); 
        float r = 800.0f; 
        
        StarData star;
        star.position = { r * sinf(phi) * cosf(theta), r * sinf(phi) * sinf(theta), r * cosf(phi) };
        star.brightness = (float)GetRandomValue(40, 100) / 100.0f;
        background_stars.push_back(star);
    }

    // 2. Neutron Star Texture & Model
    Image noise = GenImagePerlinNoise(512, 512, 0, 0, 8.0f);
    ImageColorTint(&noise, (Color){ 220, 220, 255, 255 }); 
    starTexture = LoadTextureFromImage(noise);
    UnloadImage(noise);

    Mesh sphereMesh = GenMeshSphere(1.0f, 64, 64);
    starModel = LoadModelFromMesh(sphereMesh);
    starModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = starTexture;

    bhShader = LoadShaderFromMemory(nullptr, bh_shader_code);
    resLoc = GetShaderLocation(bhShader, "resolution");
    camPosLoc = GetShaderLocation(bhShader, "camPos");
    camDirLoc = GetShaderLocation(bhShader, "camDir");
    camUpLoc = GetShaderLocation(bhShader, "camUp");
    camRightLoc = GetShaderLocation(bhShader, "camRight");
    rsLoc = GetShaderLocation(bhShader, "rs");
    timeLoc = GetShaderLocation(bhShader, "time");

    SetTargetFPS(60);
}

StarRenderer::~StarRenderer() {
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
    
    if (cameraAngleY > 1.5f) cameraAngleY = 1.5f;
    if (cameraAngleY < -1.5f) cameraAngleY = -1.5f;
    
    camera.position.x = cameraDistance * cosf(cameraAngleY) * cosf(cameraAngleX);
    camera.position.z = cameraDistance * cosf(cameraAngleY) * sinf(cameraAngleX);
    camera.position.y = cameraDistance * sinf(cameraAngleY);
}

void StarRenderer::draw_background() {
    ClearBackground((Color){ 2, 2, 8, 255 }); 
}

void StarRenderer::draw_stars_3d() {
    for(auto& star : background_stars) {
        DrawCube(star.position, 2.0f, 2.0f, 2.0f, Fade(WHITE, star.brightness));
    }
}

void StarRenderer::draw_neutron_star(double mass, const StarPhysics& physics) {
    Color coreColor = RAYWHITE;
    Color glowColor = SKYBLUE;
    
    if (mass > 1.5) { coreColor = YELLOW; glowColor = ORANGE; }
    if (mass > 1.9) { coreColor = ORANGE; glowColor = RED; }
    
    Vector3 scale = { animated_radius, animated_radius, animated_radius };
    DrawModelEx(starModel, (Vector3){0,0,0}, (Vector3){0,1,0}, physics.get_spin_angle(), scale, coreColor);

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

void StarRenderer::draw_black_hole(float rs) {
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
    if (animated_radius == 0.0f) animated_radius = target_radius;

    if (!is_black_hole) {
        if (!physics.is_paused()) {
            animated_radius += (target_radius - animated_radius) * 0.2f;
        }

        BeginDrawing();
        draw_background();
        BeginMode3D(camera);
        draw_stars_3d();
        draw_neutron_star(mass_solar, physics);
        EndMode3D();
        draw_ui(density_multiplier, target_radius, mass_solar, is_black_hole, physics);
        EndDrawing();
    } else {
        double core_mass = mass_solar * 0.80; 
        float r_s = (float)BlackHole::get_schwarzschild_radius(core_mass);

        BeginDrawing();
        ClearBackground(BLACK); 
        draw_black_hole(r_s);
        draw_ui(density_multiplier, target_radius, mass_solar, is_black_hole, physics);
        EndDrawing();
    }
}