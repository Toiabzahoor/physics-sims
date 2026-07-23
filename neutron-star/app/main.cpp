#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include "raylib.h"
#include "../include/constants.hpp"
#include "../include/eos.hpp"
#include "../include/tov.hpp"
#include "../include/rk4.hpp"

// calculates r and m
std::pair<double, double> calculate_star(double central_density, const TOVSystem& tov, const EquationOfState& eos) {
    double central_pressure = eos.returnPressure(central_density);
    std::vector<double> y = {central_pressure, 0.0}; 
    
    double r = 1.0; 
    double dr = 10.0;

    RK4::DerivativeFunc derivs = [&tov](double r, const std::vector<double>& y) {
        return tov.getDerivatives(r, y);
    };

    while (y[0] > 0.0) {
        y = RK4::step(r, y, dr, derivs);
        r += dr;
        if (r > 100000.0) break; // Failsafe
    }

    return {r / Constants::km_to_m, y[1] / Constants::M_sun};
}

int main() {
    double K = 0.025;   
    double Gamma = 2.0;
    PolytropicEoS eos(K, Gamma);
    TOVSystem tov(eos);

    double density_multiplier = 2.0; // starts at 2x density
    auto [radius_km, mass_solar] = calculate_star(Constants::rho_nuc_si * density_multiplier, tov, eos);

    // 2. Setup Raylib Window
    const int screenWidth = 1000;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Interactive TOV Neutron Star Simulation");

    // Setup 3D Camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 30.0f, 20.0f, 30.0f }; // Look from an angle
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Center of the star
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    // main loop
    while (!WindowShouldClose()) {
        bool physics_changed = false;
        
        // Adjust central density with arrow keys
        if (IsKeyDown(KEY_UP)) {
            density_multiplier += 0.05;
            physics_changed = true;
        }
        if (IsKeyDown(KEY_DOWN)) {
            density_multiplier -= 0.05;
            if (density_multiplier < 0.1) density_multiplier = 0.1; // Preventing negative density
            physics_changed = true;
        }

        if (physics_changed) {
            auto result = calculate_star(Constants::rho_nuc_si * density_multiplier, tov, eos);
            radius_km = result.first;
            mass_solar = result.second;
        }

        Color starColor = ORANGE;
        if (mass_solar > 1.8) starColor = RED;
        if (mass_solar > 2.0) starColor = MAROON;
        if (mass_solar > 2.1) starColor = DARKPURPLE; 

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
            DrawSphere((Vector3){0.0f, 0.0f, 0.0f}, (float)radius_km, starColor);
            
            DrawSphereWires((Vector3){0.0f, 0.0f, 0.0f}, (float)radius_km, 16, 16, Fade(WHITE, 0.3f));
            
            DrawGrid(50, 1.0f);
        EndMode3D();

        DrawText("Interactive TOV Neutron Star Simulator", 20, 20, 20, WHITE);
        DrawText("Controls: UP / DOWN arrows to change central density", 20, 50, 16, GRAY);
        
        std::string density_text = "Central Density: " + std::to_string(density_multiplier) + "x Nuclear Saturation";
        std::string radius_text = "Radius: " + std::to_string(radius_km) + " km";
        std::string mass_text = "Mass: " + std::to_string(mass_solar) + " Solar Masses";

        DrawText(density_text.c_str(), 20, 100, 20, GREEN);
        DrawText(radius_text.c_str(), 20, 130, 20, SKYBLUE);
        DrawText(mass_text.c_str(), 20, 160, 20, ORANGE);

        if (mass_solar > 2.1) {
            DrawText("WARNING: TOV LIMIT EXCEEDED. COLLAPSE TO BLACK HOLE.", 20, 200, 20, RED);
        }

        EndDrawing();
    }

    CloseWindow(); 
    return 0;
}