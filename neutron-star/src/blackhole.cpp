#include "../include/blackhole.hpp"
#include "rlgl.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

namespace BlackHole {

    const char* bh_shader_code = R"(
    #version 330
    
    out vec4 finalColor;
    
    uniform vec2 resolution;
    uniform vec3 camPos;
    uniform vec3 camDir;
    uniform vec3 camUp;
    uniform vec3 camRight;
    uniform float rs;
    
    void main() {
        vec2 uv = (gl_FragCoord.xy / resolution.xy - 0.5) * 2.0;
        uv.x *= resolution.x / resolution.y;
        uv *= 0.41421356; 
        
        vec3 rayDir = normalize(camDir + camRight * uv.x + camUp * uv.y);
        vec3 p = camPos;
        vec3 v = rayDir;
        
        for (int i = 0; i < 400; i++) {
            float r = length(p);
            
            if (r <= rs) {
                finalColor = vec4(0.0, 0.0, 0.0, 1.0);
                return;
            }
            if (r > max(2000.0, length(camPos) * 1.5)) {
                break;
            }
            
            float dt = max(r * 0.05, 0.05);
            vec3 gravity = -p * (0.8 * rs / max(r * r * r, 0.001));
            v = normalize(v + gravity * dt);
            p += v * dt;
        }
        
        vec2 st = gl_FragCoord.xy / resolution.xy;
        float star = fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
        if (star > 0.998) {
            float intensity = (star - 0.998) * 500.0;
            finalColor = vec4(vec3(intensity), 1.0);
        } else {
            finalColor = vec4(0.01, 0.01, 0.02, 1.0);
        }
    }
    )";

    bool is_collapsed(double mass_solar) {
        return mass_solar > TOV_LIMIT_SOLAR;
    }

    double get_schwarzschild_radius(double mass_solar) {
        return 2.95 * mass_solar;
    }

    Visuals::Visuals() : is_initialized(false) {}

    Visuals::~Visuals() {
        cleanup();
    }

    void Visuals::init() {
        if (is_initialized) return;

        Image img = GenImageGradientRadial(16, 16, 0.4f, WHITE, BLANK);
        particleTex = LoadTextureFromImage(img);
        UnloadImage(img);

        for (int i = 0; i < 30000; i++) {
            DiskParticle p;
            
            float u = (float)GetRandomValue(0, 1000) / 1000.0f;
            p.distance = 1.0f + 12.0f * pow(u, 3.0f); 
            
            float raw_angle = (float)GetRandomValue(0, 360) * (PI / 180.0f);
            
            p.angle = raw_angle + sin(raw_angle * 2.0f) * 0.6f; 
            p.distance *= (1.0f + 0.35f * cos(p.angle)); 
            
            float thickness = 0.01f + (p.distance * 0.015f);
            p.y_offset = ((float)GetRandomValue(-100, 100) / 100.0f) * thickness;
            
            p.base_speed = 0.6f / sqrt(p.distance); 
            p.size = (float)GetRandomValue(2, 6) / 10.0f; 
            
            particles.push_back(p);
        }
        is_initialized = true;
    }

    void Visuals::cleanup() {
        if (is_initialized) {
            UnloadTexture(particleTex);
            particles.clear();
            is_initialized = false;
        }
    }

    void Visuals::update(float dt, float horizon_radius) {
        if (!is_initialized || horizon_radius <= 0.0f) return;

        for (auto& p : particles) {
            p.angle += p.base_speed * dt;
            if (p.angle > 2.0f * PI) p.angle -= 2.0f * PI;

            if (p.distance <= 2.0f) {
                float plunge_speed = 1.0f * (2.0f - p.distance); 
                p.distance -= plunge_speed * dt;
                
                if (p.distance <= 0.95f) {
                    p.distance = 12.0f * (1.0f + 0.35f * cos(p.angle));
                    p.angle = (float)GetRandomValue(0, 360) * (PI / 180.0f);
                }
            } else {
                p.distance -= 0.02f * dt; 
            }
        }
    }

    void Visuals::draw(float horizon_radius, Camera3D camera, float alpha_multiplier) {
        if (!is_initialized || horizon_radius <= 0.0f || alpha_multiplier <= 0.0f) return;

        rlDisableDepthMask(); 
        BeginBlendMode(BLEND_ADDITIVE);

        for (const auto& p : particles) {
            float actual_r = p.distance * horizon_radius;
            
            Vector3 pos = {
                actual_r * cos(p.angle),
                p.y_offset * horizon_radius,
                actual_r * sin(p.angle)
            };

            Color p_color;
            float temp_factor = 1.0f / p.distance;
            
            if (temp_factor > 0.4f) {
                p_color = {255, 230, 180, 255};
            } else if (temp_factor > 0.2f) {
                p_color = {255, 120, 30, 255};
            } else {
                p_color = {150, 20, 10, 255};
            }

            Vector3 to_cam = { camera.position.x - pos.x, camera.position.y - pos.y, camera.position.z - pos.z };
            float cam_dist = sqrt(to_cam.x*to_cam.x + to_cam.y*to_cam.y + to_cam.z*to_cam.z);
            to_cam.x /= cam_dist; to_cam.z /= cam_dist; 

            Vector3 vel = { -sin(p.angle), 0.0f, cos(p.angle) };
            float alignment = (vel.x * to_cam.x + vel.z * to_cam.z);

            float doppler_shift = 1.0f + (alignment * 0.7f);
            if (doppler_shift < 0.1f) doppler_shift = 0.1f;

            if (p.distance < 1.2f) doppler_shift *= (p.distance - 1.0f) * 5.0f;

            float r_f = (float)p_color.r * doppler_shift;
            float g_f = (float)p_color.g * doppler_shift;
            float b_f = (float)p_color.b * doppler_shift;
            float a_f = 255.0f * std::min(temp_factor * 1.8f, 1.0f) * doppler_shift * alpha_multiplier;

            p_color.r = (unsigned char)std::min(r_f, 255.0f);
            p_color.g = (unsigned char)std::min(g_f, 255.0f);
            p_color.b = (unsigned char)std::min(b_f, 255.0f);
            p_color.a = (unsigned char)std::min(a_f, 255.0f);

            DrawBillboard(camera, particleTex, pos, p.size, p_color);
        }

        EndBlendMode();
        rlEnableDepthMask();
    }
}