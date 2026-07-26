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
    uniform float time;
    
    // Smooth noise functions
    float hash(vec2 p) {
        p = fract(p * vec2(234.34, 435.345));
        p += dot(p, p + 19.19);
        return fract(p.x * p.y);
    }
    
    float smooth_noise(vec2 p) {
        vec2 i = floor(p);
        vec2 f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        float a = hash(i);
        float b = hash(i + vec2(1.0, 0.0));
        float c = hash(i + vec2(0.0, 1.0));
        float d = hash(i + vec2(1.0, 1.0));
        return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
    }
    
    float fbm(vec2 p) {
        float val = 0.0;
        float amp = 0.5;
        float freq = 1.0;
        for (int i = 0; i < 5; i++) {
            val += amp * smooth_noise(p * freq);
            freq *= 2.0;
            amp *= 0.5;
        }
        return val;
    }
    
    // Temperature-based color: white-hot inner, orange mid, red outer
    vec3 disk_color(float temperature) {
        float t = clamp(temperature, 0.0, 1.0);
        vec3 cool = vec3(0.6, 0.1, 0.02);
        vec3 warm = vec3(1.0, 0.6, 0.1);
        vec3 hot = vec3(1.0, 0.95, 0.8);
        vec3 very_hot = vec3(1.0, 1.0, 1.0);
        
        if (t < 0.33) {
            return mix(cool, warm, t / 0.33);
        } else if (t < 0.66) {
            return mix(warm, hot, (t - 0.33) / 0.33);
        } else {
            return mix(hot, very_hot, (t - 0.66) / 0.34);
        }
    }
    
    // Smooth accretion disk sampling
    vec3 sample_disk(vec2 pos, float time_val) {
        float r = length(pos);
        float angle = atan(pos.y, pos.x);
        
        float inner = 2.5;
        float outer = 14.0;
        if (r < inner || r > outer) return vec3(0.0);
        
        // Smooth radial profile: peaks near inner edge, falls off
        float normalized_r = (r - inner) / (outer - inner);
        float density = exp(-normalized_r * 3.0) * 2.5;
        density = min(density, 3.5);
        
        // Smooth spiral arms
        float spiral = sin(angle * 3.0 - r * 2.5 + time_val * 8.0) * 0.15 + 0.85;
        
        // Very smooth turbulence (low frequency only)
        vec2 turb_uv = pos * 0.8 + vec2(time_val * 2.0, 0.0);
        float turb = fbm(turb_uv) * 0.5 + 0.5;
        
        density *= spiral * turb * 2.5;
        density = max(density, 0.0);
        
        // Temperature: white-hot at inner edge, cooling outward
        float temp = pow(inner / max(r, inner), 0.9);
        temp = clamp(temp, 0.0, 1.0);
        
        vec3 color = disk_color(temp) * density * (0.6 + temp * 0.4);
        return color;
    }
    
    // Ray-plane intersection for disk
    float intersect_disk_plane(vec3 ro, vec3 rd, float t_min, float t_max) {
        if (abs(rd.y) < 1e-6) return -1.0;
        float t = -ro.y / rd.y;
        if (t < t_min || t > t_max) return -1.0;
        return t;
    }
    
    void main() {
        vec2 uv = (gl_FragCoord.xy / resolution.xy - 0.5) * 2.0;
        uv.x *= resolution.x / resolution.y;
        uv *= 0.41421356;
        
        vec3 rayDir = normalize(camDir + camRight * uv.x + camUp * uv.y);
        vec3 p = camPos;
        vec3 v = rayDir;
        
        vec3 accretion_color = vec3(0.0);
        float accretion_alpha = 0.0;
        float min_r_over_rs = 1e10;
        bool hit_horizon = false;
        bool ray_escaped = false;
        
        for (int i = 0; i < 800; i++) {
            float r = length(p);
            float r_over_rs = r / max(rs, 0.001);
            
            if (r_over_rs < min_r_over_rs) {
                min_r_over_rs = r_over_rs;
            }
            
            if (r <= rs) {
                hit_horizon = true;
                break;
            }
            //reduced to 1000 for better performance
            if (r > max(1000.0, length(camPos) * 2.0)) {
                ray_escaped = true;
                break;
            }
            
            float dt = max(r * 0.015, 0.01);
            
            // Check for disk intersection along this step
            float t_disk = intersect_disk_plane(p, v, 0.0, dt);
            if (t_disk > 0.0) {
                vec3 hit_pos = p + v * t_disk;
                float hit_r = length(hit_pos.xz) / max(rs, 0.001);
                
                if (hit_r > 2.0 && hit_r < 15.0) {
                    vec2 disk_uv = hit_pos.xz / max(rs, 0.001);
                    vec3 disk_col = sample_disk(disk_uv, time);
                    
                    if (length(disk_col) > 0.001) {
                        // Smooth orbital velocity for doppler
                        vec3 orbital_dir = normalize(vec3(-hit_pos.z, 0.0, hit_pos.x));
                        float beta = sqrt(1.0 / max(2.0 * hit_r - 3.0, 0.1));
                        
                        // Doppler beaming: approaching side gets brighter and bluer
                        float cos_angle = dot(orbital_dir, normalize(-v));
                        float gamma = 1.0 / sqrt(1.0 - beta * beta);
                        float doppler = 1.0 / (gamma * (1.0 - beta * cos_angle));
                        float beaming = pow(doppler, 3.0);
                        
                        // Gravitational redshift near horizon
                        float redshift = sqrt(max(1.0 - 1.5 / hit_r, 0.01));
                        
                        vec3 final_hit = disk_col * beaming * redshift;
                        
                        // Accumulate
                        float alpha = min(length(final_hit) * 0.4, 1.0);
                        accretion_color += final_hit * (1.0 - accretion_alpha);
                        accretion_alpha += alpha * (1.0 - accretion_alpha);
                        
                        if (accretion_alpha > 0.95) break;
                    }
                }
            }
            
            // Gravity step
            vec3 h = cross(p, v);
            vec3 gravity = -p * (1.5 * rs * dot(h, h) / max(pow(r, 5.0), 0.0001));
            v = normalize(v + gravity * dt);
            p += v * dt;
        }
        
        if (hit_horizon) {
            // Bright photon ring at ~1.5 Rs
            float photon_ring = exp(-abs(min_r_over_rs - 1.5) * 80.0);
            vec3 ring_glow = vec3(1.0, 0.9, 0.7) * photon_ring * 2.0;
            
            // Inner glow
            vec3 inner_glow = vec3(1.0, 0.7, 0.3) * exp(-min_r_over_rs * 4.0) * 0.4;
            
            finalColor = vec4(ring_glow + inner_glow, 1.0);
            return;
        }
        
        // Background: subtle starfield
        vec2 st = gl_FragCoord.xy / resolution.xy;
        float star_seed = hash(st * 100.0);
        vec3 bg = vec3(0.003, 0.003, 0.005);
        
        if (star_seed > 0.997) {
            float intensity = (star_seed - 0.997) * 333.0;
            float color_variation = hash(st * 200.0 + 1.0);
            bg = vec3(intensity) * (0.7 + 0.3 * color_variation);
        }
        
        // Subtle galaxy band
        float band = sin(st.y * 12.0 + st.x * 4.0) * 0.5 + 0.5;
        band *= sin(st.x * 7.0 - st.y * 5.0) * 0.5 + 0.5;
        band = pow(band, 3.0) * 0.01;
        bg += vec3(0.08, 0.06, 0.12) * band;
        
        // Blend accretion disk over background
        if (accretion_alpha > 0.01) {
            vec3 final_accretion = accretion_color / max(accretion_alpha, 0.01);
            finalColor = vec4(mix(bg, final_accretion, min(accretion_alpha, 0.85)), 1.0);
        } else {
            finalColor = vec4(bg, 1.0);
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
            
            p.base_speed = 2.5f / sqrt(p.distance); 
            p.size = (float)GetRandomValue(1, 4) / 10.0f; 
            
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
            
            // Skip particles inside or too close to event horizon
            if (p.distance < 1.1f) continue;
            
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