#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"

const unsigned int TARGET_FPS = 50;
float dt = 1.0f / TARGET_FPS;
float time = 0;
Vector2 launchPosition = { 250, 250 };
float launchSpeed = 500;
float launchAngle = 45;
Vector2 endPos;

void update()
{
    time += dt;
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        launchPosition = GetMousePosition();
    endPos = { launchPosition.x + launchSpeed * cosf(launchAngle * DEG2RAD), launchPosition.y + launchSpeed * sinf(launchAngle * DEG2RAD) };
}
void draw()
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("Michael McKall 101551503", 10, float(GetScreenHeight() - 30), 20, LIGHTGRAY);
    DrawText(TextFormat("Change launchPosition by right clicking. launchPosition: {%08f, %08f}", launchPosition.x, launchPosition.y), 10, 5, 20, LIGHTGRAY);
    GuiSliderBar(Rectangle{ 100, 25, 1000, 10 }, "launchSpeed", TextFormat("%.2f", launchSpeed), &launchSpeed, 0, 500);
    GuiSliderBar(Rectangle{ 100, 50, 1000, 10 }, "launchAngle", TextFormat("%.2f", launchAngle), &launchAngle, 0, 360);
    // GuiSliderBar(Rectangle{ 10, 15, 1000, 20 }, "", TextFormat("%.2f", time), &time, 0, 240);
    DrawText(TextFormat("FPS: %.2i, TIME: %.2f", TARGET_FPS, time), GetScreenWidth() - 175, 75, 15, LIGHTGRAY);

    DrawLine(launchPosition.x, launchPosition.y, endPos.x, endPos.y, RED);

    EndDrawing();
}
int main()
{
    InitWindow(1200, 800, "Physics-1");
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose())
    {
        update();
        draw();
    }

    CloseWindow();
    return 0;
}
