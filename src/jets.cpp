#include "../include/jets.hpp"
#include "raymath.h"
#include "rlgl.h"
#include <algorithm>
#include <cmath>

PolarJets::PolarJets() : is_initialized(false) {}

PolarJets::~PolarJets() {
    cleanup();
}

void PolarJets::init() {
    if (is_initialized) return;
    
    Image img = GenImageGradientRadial(64, 64, 0.0f, WHITE, BLANK);
    jetTex = LoadTextureFromImage(img);
    UnloadImage(img);
    
    is_initialized = true;
}

void PolarJets::cleanup() {
    if (is_initialized) {
        UnloadTexture(jetTex);
        particles.clear();
        is_initialized = false;
    }
}

void PolarJets::update(float dt, bool is_black_hole, Vector3 axis, float escape_velocity) {
    if (!is_initialized) return;

    for (auto& p : particles) {
        p.position = Vector3Add(p.position, Vector3Scale(p.velocity, dt));
        
        Vector3 proj_on_axis = Vector3Scale(axis, Vector3DotProduct(p.position, axis));
        Vector3 radial_vec = Vector3Subtract(p.position, proj_on_axis);
        float dist_from_axis = Vector3Length(radial_vec);
        
        if (dist_from_axis > 0.05f) {
            Vector3 pinch = Vector3Scale(Vector3Normalize(radial_vec), -15.0f * dt);
            p.velocity = Vector3Add(p.velocity, pinch);
        }

        p.life -= dt;
    }

    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const JetParticle& p) { return p.life <= 0.0f; }), particles.end());

    if (!is_black_hole) {
        int new_particles = 250; 

        for (int i = 0; i < new_particles; i++) {
            JetParticle p;
            
            float sign = (GetRandomValue(0, 1) == 0) ? 1.0f : -1.0f;
            float spread = 2.5f; 
            
            Vector3 random_dir = {
                (float)GetRandomValue(-100, 100) / 100.0f * spread,
                (float)GetRandomValue(-100, 100) / 100.0f * spread,
                (float)GetRandomValue(-100, 100) / 100.0f * spread
            };

            float jet_speed = 180.0f + (float)GetRandomValue(-20, 20);
            Vector3 base_vel = Vector3Scale(axis, sign * jet_speed);
            
            p.velocity = Vector3Add(base_vel, Vector3Scale(random_dir, 6.0f));

            p.position = Vector3Scale(axis, sign * 2.0f);
            p.position = Vector3Add(p.position, random_dir);

            p.max_life = (float)GetRandomValue(50, 180) / 100.0f;
            p.life = p.max_life;
            p.size = (float)GetRandomValue(25, 60) / 10.0f; 

            float temp = (float)GetRandomValue(0, 100) / 100.0f;
            if (temp > 0.80f) p.color = {255, 255, 255, 255};
            else if (temp > 0.40f) p.color = {100, 200, 255, 255};
            else p.color = {20, 50, 255, 255};

            particles.push_back(p);
        }
    }
}

void PolarJets::draw(Camera3D camera, float alpha_multiplier) {
    if (!is_initialized || alpha_multiplier <= 0.0f) return;

    rlDisableDepthMask();
    rlDisableBackfaceCulling();
    BeginBlendMode(BLEND_ADDITIVE);

    float beta = 0.5f; 
    float gamma = 1.0f / std::sqrt(1.0f - beta * beta);

    for (const auto& p : particles) {
        float life_ratio = p.life / p.max_life;
        
        Vector3 view_dir = Vector3Normalize(Vector3Subtract(camera.position, p.position));
        Vector3 vel_dir = Vector3Normalize(p.velocity);
        float cos_theta = Vector3DotProduct(vel_dir, view_dir);
        
        float doppler_factor = 1.0f / (gamma * (1.0f - beta * cos_theta));
        
        float beaming_boost = std::pow(doppler_factor, 1.2f);
        beaming_boost = std::max(0.35f, beaming_boost); 
        
        Color c = p.color;
        float alpha = life_ratio < 0.2f ? (life_ratio / 0.2f) : 1.0f;
        alpha *= (life_ratio > 0.8f ? ((1.0f - life_ratio) / 0.2f) : 1.0f);

        float boosted_alpha = alpha * beaming_boost * alpha_multiplier;
        
        float alpha_attenuation = 0.6f;
        c.a = (unsigned char)std::min(255.0f, 255.0f * boosted_alpha * alpha_attenuation);
        
        c.r = (unsigned char)std::min(255.0f, c.r * beaming_boost);
        c.g = (unsigned char)std::min(255.0f, c.g * beaming_boost);
        c.b = (unsigned char)std::min(255.0f, c.b * beaming_boost);

        float current_size = p.size * (1.0f + (1.0f - life_ratio) * 2.5f);

        if (c.a > 2) {
            DrawBillboard(camera, jetTex, p.position, current_size, c);
        }
    }

    EndBlendMode();
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}