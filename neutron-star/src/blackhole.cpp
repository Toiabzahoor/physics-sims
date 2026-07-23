#include "../include/blackhole.hpp"
#include <cmath>

namespace BlackHole {

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

        for (int i = 0; i < 12000; i++) {
            DiskParticle p;
            
            float u = (float)GetRandomValue(0, 1000) / 1000.0f;
            p.distance = 1.0f + 14.0f * pow(u, 2.0f); 
            p.angle = (float)GetRandomValue(0, 360) * (PI / 180.0f);
            
            float thickness = 0.05f + (p.distance * 0.02f);
            p.y_offset = ((float)GetRandomValue(-100, 100) / 100.0f) * thickness;
            
            // Keplerian velocity approximation 
            p.base_speed = 1.5f / sqrt(p.distance); 
            p.size = (float)GetRandomValue(4, 12) / 10.0f;
            
            particles.push_back(p);
        }

        is_initialized = true;
    }

    void Visuals::cleanup() {
        particles.clear();
        is_initialized = false;
    }

    void Visuals::update(float dt, float horizon_radius) {
        if (!is_initialized || horizon_radius <= 0.0f) return;

        for (auto& p : particles) {
            // Apply orbital rotation
            p.angle += p.base_speed * dt;
            if (p.angle > 2.0f * PI) p.angle -= 2.0f * PI;

            // ISCO & Plunge Region Physics
            if (p.distance <= 3.0f) {
                float plunge_speed = 0.8f * (3.0f - p.distance); 
                p.distance -= plunge_speed * dt;
                
                if (p.distance <= 1.0f) {
                    p.distance = 15.0f; 
                    p.angle = (float)GetRandomValue(0, 360) * (PI / 180.0f);
                }
            } else {
                p.distance -= 0.1f * dt;
            }
        }
    }

    void Visuals::draw(float horizon_radius, Vector3 camera_pos) {
        if (!is_initialized || horizon_radius <= 0.0f) return;

        DrawSphereEx((Vector3){0, 0, 0}, horizon_radius, 64, 64, BLACK);

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
            
            if (temp_factor > 0.5f) {
                p_color = WHITE;
            } else if (temp_factor > 0.2f) {
                p_color = ORANGE;
            } else {
                p_color = RED;
            }

            // Dynamic Doppler Beaming
            Vector3 to_cam = { camera_pos.x - pos.x, camera_pos.y - pos.y, camera_pos.z - pos.z };
            float cam_dist = sqrt(to_cam.x*to_cam.x + to_cam.y*to_cam.y + to_cam.z*to_cam.z);
            to_cam.x /= cam_dist; to_cam.z /= cam_dist; // Normalize in XZ plane

            Vector3 vel = { -sin(p.angle), 0.0f, cos(p.angle) };

            float alignment = (vel.x * to_cam.x + vel.z * to_cam.z);
            float doppler_shift = 1.0f + (alignment * 0.85f); 
            if (doppler_shift < 0.1f) doppler_shift = 0.1f;

            if (p.distance < 1.2f) {
                doppler_shift *= (p.distance - 1.0f) * 5.0f;
            }

            p_color.r = (unsigned char)(p_color.r * doppler_shift);
            p_color.g = (unsigned char)(p_color.g * doppler_shift);
            p_color.b = (unsigned char)(p_color.b * doppler_shift);
            p_color.a = (unsigned char)(200 * temp_factor * doppler_shift);

            DrawCube(pos, p.size, p.size, p.size, p_color);
        }

        EndBlendMode();
    }
}