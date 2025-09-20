#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

const unsigned int TARGET_FPS = 50;
int main()
{
    Vector2 launchPosition = { 250, 250 };
    float launchSpeed = 500;
    float launchAngle = 45;
    Vector2 endPos;
    InitWindow(1200, 800, "Physics-1");
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(BLACK);
            
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                launchPosition = GetMousePosition();

            DrawText(TextFormat("Change launchPosition by right clicking. launchPosition: {%08f, %08f}", launchPosition.x, launchPosition.y), 10, 5, 20, LIGHTGRAY);
            GuiSliderBar(Rectangle{ 100, 25, 1000, 10 }, "launchSpeed", TextFormat("%.2f", launchSpeed), &launchSpeed, 0, 500);
            GuiSliderBar(Rectangle{ 100, 50, 1000, 10 }, "launchSpeed", TextFormat("%.2f", launchAngle), &launchAngle, 0, 360);

            endPos = { launchPosition.x + launchSpeed * cosf(launchAngle * PI/180), launchPosition.y + launchSpeed * sinf(launchAngle * PI/180)};

            DrawLine(launchPosition.x, launchPosition.y, endPos.x, endPos.y, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
