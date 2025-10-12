/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"
#include "vector"

const unsigned int TARGET_FPS = 50; //frames/second

float launchSpeed = 100;
float launchAngle = 0;
float gravMag = 100;
float gravDir = 270;

Vector2 launchPosition = { 500, 500 };

class physicsSimulation
{
public:
	float deltaTime = 1.0f / TARGET_FPS; //seconds/frame
	float time = 0.0f;
	Vector2 gravity = { launchSpeed * (float)cos(launchAngle * DEG2RAD), -launchSpeed * (float)sin(launchAngle * DEG2RAD) };
	class physicsBody
	{
	public:
		Vector2 position = { 0,0 };
		Vector2 velocity = { 0,0 };
		Vector2 drag = { 0, 0 };
		float mass = 1;
		virtual void draw()
		{
			DrawText("Nothing to draw here!", position.x, position.y, 5, RED);
		};
	};
};
class physicsCircle : public physicsSimulation::physicsBody
{
public:
	float radius = 15;
	Color color = GREEN;
	physicsCircle(Vector2 conPosition, Vector2 conVelocity, float conRadius)
	{
		position = conPosition;
		velocity = conVelocity;
		radius = conRadius;
	}	
	void draw() override
	{
		DrawCircle(position.x, position.y, radius, color);
		DrawLineEx(position, position + velocity, 1, color);
	}
};

physicsSimulation physicsSimulationObject;

std::vector<physicsCircle*> pObjects;

void collision()
{
	for (int i = 0; i < pObjects.size(); i++)
	{
		pObjects[i]->color = GREEN;
	}
	for (int i = 0; i < pObjects.size(); i++)
	{
		for (int j = 0; j < pObjects.size(); j++)
		{
			if (i != j)
			{
				float sumRadii = pObjects[i]->radius + pObjects[j]->radius;
				Vector2 displacement = pObjects[j]->position - pObjects[i]->position;

				float distance = Vector2Length(displacement);

				if (distance < sumRadii)
				{
					pObjects[i]->color = RED;
					pObjects[j]->color = RED;
				}
			}
		}
	}
}
void deletion()
{
	for (int i = 0; i < pObjects.size(); i++)
	{
		if (pObjects[i]->position.y > GetScreenHeight()
			|| pObjects[i]->position.y < 0
			|| pObjects[i]->position.x > GetScreenWidth()
			|| pObjects[i]->position.x < 0)
		{
			delete* (pObjects.begin() + i);
			pObjects.erase(pObjects.begin() + i);
			i--;
		}
	}
}

//Changes world state
void update()
{
	physicsSimulationObject.time += physicsSimulationObject.deltaTime;
	physicsSimulationObject.gravity = { gravMag * (float)cos(gravDir * DEG2RAD), -gravMag * (float)sin(gravDir * DEG2RAD) };
	for (int i = 0; i < pObjects.size(); i++)
	{
		pObjects[i]->position += pObjects[i]->velocity * physicsSimulationObject.deltaTime;
		pObjects[i]->velocity += physicsSimulationObject.gravity * physicsSimulationObject.deltaTime;
	}
	//vel = change in position / time, therefore change in position = vel * time
	
	

	//accel = deltaV / time (change in velocity over time) therefore deltaV = accel * time
	

	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        launchPosition = GetMousePosition();
	if (IsKeyPressed(KEY_SPACE))
	{
		Vector2 velocity = {launchSpeed * (float)cos(launchAngle * DEG2RAD), -launchSpeed * (float)sin(launchAngle * DEG2RAD)};
		float newRadius = 15;
		physicsCircle* newCircle = new physicsCircle(launchPosition, velocity, newRadius);
		pObjects.push_back(newCircle);
	}
	collision();
	deletion();
}

//Display world state
void draw()
{
	BeginDrawing();
	ClearBackground(BLACK);
	DrawText("Michael McKall 101551503", 10, float(GetScreenHeight() - 30), 20, LIGHTGRAY);
	DrawText(TextFormat("Change launchPosition by right clicking. launchPosition: {%08f, %08f}", launchPosition.x, launchPosition.y), 10, 5, 20, LIGHTGRAY);

	GuiSliderBar(Rectangle{ 10, 40, 1000, 20 }, "", TextFormat("%.2f", physicsSimulationObject.time), &physicsSimulationObject.time, 0, 240);

	GuiSliderBar(Rectangle{ 10, 80, 500, 30 }, "Speed", TextFormat("Speed: %.0f", launchSpeed), &launchSpeed, -1000, 1000);

	GuiSliderBar(Rectangle{ 10, 120, 500, 30 }, "Angle", TextFormat("Angle: %.0f Degrees", launchAngle), &launchAngle, 0, 360);

	GuiSliderBar(Rectangle{ 10, 160, 500, 30 }, "Gravity Magnitude", TextFormat("Magnitude: %.0f", gravMag), &gravMag, -1000, 1000);

	GuiSliderBar(Rectangle{ 10, 200, 500, 30 }, "Gravity Direction", TextFormat("Direction: %.0f Degrees", gravDir), &gravDir, 0, 360);

	int objectCount = pObjects.size();
	DrawText(TextFormat("Object Count: %i", objectCount), GetScreenWidth() - 300, 100, 30, LIGHTGRAY);
	DrawText(TextFormat("T: %6.2f", physicsSimulationObject.time), GetScreenWidth() - 140, 10, 30, LIGHTGRAY);

	Vector2 velocity = { launchSpeed * cos(launchAngle * DEG2RAD), -launchSpeed * sin(launchAngle * DEG2RAD)};

	DrawLineEx(launchPosition, launchPosition + velocity, 3, RED);
	for (int i = 0; i < pObjects.size(); i++)
	{
		pObjects[i]->draw();
	}

	EndDrawing();

}

int main()
{
	InitWindow(InitialWidth, InitialHeight, "GAME2005 Michael McKall 101551503");
	SetTargetFPS(TARGET_FPS);

	while (!WindowShouldClose()) // Loops TARGET_FPS times per second
	{
		update();
		draw();
	}

	CloseWindow();
	return 0;
}