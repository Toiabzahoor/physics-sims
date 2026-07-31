#include "raylib.h"
#include "renderer.hpp"
#include "star_physics.hpp"
#include "blackhole.hpp"

enum class AppState {
    MENU,
    SIMULATION
};

bool DrawElegantButton(Rectangle bounds, const char* text, bool isHovered) {
    Color bgColor = isHovered ? Fade(SKYBLUE, 0.2f) : Fade(DARKGRAY, 0.2f);
    Color textColor = isHovered ? WHITE : LIGHTGRAY;
    Color lineColor = isHovered ? SKYBLUE : Fade(GRAY, 0.5f);

    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 1.5f, lineColor);

    int textWidth = MeasureText(text, 22);
    DrawText(text, bounds.x + bounds.width / 2 - textWidth / 2, bounds.y + bounds.height / 2 - 11, 22, textColor);

    return isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    StarRenderer renderer(screenWidth, screenHeight, "Physics Simulations");
    SetExitKey(KEY_NULL);

    AppState state = AppState::MENU;
    StarPhysics physics;

    double density = 1.0;
    double radius = 12.0;
    double mass = 1.4;
    bool is_black_hole = false;

    while (!renderer.should_close()) {
        if (state == AppState::MENU) {
            BeginDrawing();
            ClearBackground((Color){ 10, 10, 15, 255 });

            const char* title = "ASTROPHYSICS SIMULATOR";
            int titleWidth = MeasureText(title, 40);
            DrawText(title, screenWidth / 2 - titleWidth / 2, 180, 40, RAYWHITE);

            const char* subtitle = "Select a module to begin";
            int subWidth = MeasureText(subtitle, 20);
            DrawText(subtitle, screenWidth / 2 - subWidth / 2, 240, 20, GRAY);

            Rectangle nsButton = { (float)screenWidth / 2 - 175, 360, 350, 60 };
            bool nsHovered = CheckCollisionPointRec(GetMousePosition(), nsButton);

            if (DrawElegantButton(nsButton, "Neutron Star / Black Hole", nsHovered)) {
                state = AppState::SIMULATION;
            }

            Rectangle exitButton = { (float)screenWidth / 2 - 175, 440, 350, 60 };
            bool exitHovered = CheckCollisionPointRec(GetMousePosition(), exitButton);

            if (DrawElegantButton(exitButton, "Exit", exitHovered)) {
                break;
            }

            const char* footnote = "Press ESC inside a simulation to return here";
            DrawText(footnote, screenWidth / 2 - MeasureText(footnote, 15) / 2, screenHeight - 40, 15, Fade(GRAY, 0.6f));

            EndDrawing();
        } else if (state == AppState::SIMULATION) {
            renderer.update_input(physics);

            if (IsKeyDown(KEY_W)) {
                density += 0.01;
            }
            if (IsKeyDown(KEY_S)) {
                density -= 0.01;
                if (density < 0.1) density = 0.1;
            }

            // Solve the TOV equations via RK4 to get physically accurate radius and mass
            physics.update_hydrostatic_equilibrium(density, radius, mass);
            is_black_hole = BlackHole::is_collapsed(mass);

            if (IsKeyPressed(KEY_ESCAPE)) {
                state = AppState::MENU;
            }

            physics.update(GetFrameTime());
            renderer.render_frame(density, radius, mass, is_black_hole, physics);
        }
    }

    return 0;
}