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

enum physicsShape
{
	CIRCLE,
	HALFSPACE
};

class physicsSimulation
{
public:
	float deltaTime = 1.0f / TARGET_FPS; //seconds/frame
	float time = 0.0f;
	Vector2 gravity = { launchSpeed * (float)cos(launchAngle * DEG2RAD), -launchSpeed * (float)sin(launchAngle * DEG2RAD) };
	class physicsBody
	{
	public:
		bool staticBody = false;

		Vector2 position = { 0,0 };
		Vector2 velocity = { 0,0 };
		Vector2 drag = { 0, 0 };
		float mass = 1;
		Color color = GREEN;
		virtual void draw()
		{
			DrawText("Nothing to draw here!", position.x, position.y, 5, RED);
		};

		virtual physicsShape Shape() = 0;
	};
};

class physicsCircle : public physicsSimulation::physicsBody
{
public:
	float radius = 15;
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
	physicsShape Shape() override
	{
		return CIRCLE;
	}
};

class physicsHalfspace : public physicsSimulation::physicsBody
{
private:
	float rotation = 0;
	Vector2 normal = { 0, -1 };
public:
	void setRotation(float rotationInDegrees)
	{
		rotation = rotationInDegrees;
		normal = Vector2Rotate({ 0, -1 }, rotation * DEG2RAD);
	}

	float getRotation()
	{
		return rotation;
	}
	Vector2 getNormal()
	{
		return normal;
	}
	void draw() override
	{
		DrawCircle(position.x, position.y, 8, color);
		DrawLineEx(position, position + normal * 30, 1, color);

		Vector2 parallelToSurface = Vector2Rotate(normal, 90 * DEG2RAD);
		DrawLineEx(position - parallelToSurface * 4000, position + parallelToSurface * 4000, 1, color);
	}
	physicsShape Shape() override
	{
		return HALFSPACE;
	}
};

physicsSimulation physicsSimulationObject;
physicsHalfspace halfspace;
physicsHalfspace halfspace2;

std::vector<physicsSimulation::physicsBody*> pObjects;

bool circleCircleCollision(physicsCircle* circleA, physicsCircle* circleB)
{
	float sumRadii = circleA->radius + circleB->radius;
	Vector2 displacement = circleB->position - circleA->position;

	float distance = Vector2Length(displacement);

	return (distance < sumRadii) ? true : false;
}

bool circleCircleCollisionResponse(physicsCircle* circleA, physicsCircle* circleB)
{
	float sumRadii = circleA->radius + circleB->radius;
	Vector2 displacement = circleB->position - circleA->position;

	float distance = Vector2Length(displacement);
	float overlap = sumRadii - distance;

	if (overlap > 0)
	{
		Vector2 normalAtoB;
		if (!distance)
			normalAtoB = { 0, 1 };
		else
			normalAtoB = displacement / distance;
		Vector2 mtv = normalAtoB * overlap; // Minimum translation vector (to push apart for collision)
		circleA->position -= mtv * 0.5f;
		circleB->position += mtv * 0.5f;
		return true;
	}
	else
		return false;
}

bool circleHalfspaceCollision(physicsCircle* circle, physicsHalfspace* halfspace)
{
	Vector2 displacementToCircle = circle->position - halfspace->position;

	float dotProduct = Vector2DotProduct(displacementToCircle, halfspace->getNormal());
	Vector2 vectorProjection = halfspace->getNormal() * dotProduct;
	//float distance = Vector2Length(displacementToCircle);

	DrawLineEx(circle->position, circle->position - vectorProjection, 1, GRAY);

	Vector2 midpoint = circle->position - vectorProjection * 0.5f;
	DrawText(TextFormat("D: %3.0f", dotProduct), midpoint.x, midpoint.y, 30, LIGHTGRAY);

	return dotProduct < circle->radius;
}

bool circleHalfspaceCollisionResponse(physicsCircle* circle, physicsHalfspace* halfspace)
{
	Vector2 displacementToCircle = circle->position - halfspace->position;

	float dotProduct = Vector2DotProduct(displacementToCircle, halfspace->getNormal());
	Vector2 vectorProjection = halfspace->getNormal() * dotProduct;
	//float distance = Vector2Length(displacementToCircle);

	//DrawLineEx(circle->position, circle->position - vectorProjection, 1, GRAY);

	//Vector2 midpoint = circle->position - vectorProjection * 0.5f;
	//DrawText(TextFormat("D: %3.0f", dotProduct), midpoint.x, midpoint.y, 30, LIGHTGRAY);

	float overlap = circle->radius - dotProduct;

	if (overlap > 0)
	{
		Vector2 mtv = halfspace->getNormal() * overlap;
		circle->position += mtv;
		return true;
	}
	else
	{
		return false;
	}
}

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
				physicsSimulation::physicsBody* objectA = pObjects[i];
				physicsSimulation::physicsBody* objectB = pObjects[j];

				physicsShape shapeA = objectA->Shape();
				physicsShape shapeB = objectB->Shape();

				bool didOverlap = false;

				if (shapeA == CIRCLE && shapeB == CIRCLE)
					didOverlap = circleCircleCollisionResponse((physicsCircle*)objectA, (physicsCircle*)objectB);

				else if (shapeA == CIRCLE && shapeB == HALFSPACE)
					didOverlap = circleHalfspaceCollisionResponse((physicsCircle*)objectA, (physicsHalfspace*)objectB);

				else if (shapeB == CIRCLE && shapeA == HALFSPACE)
					didOverlap = circleHalfspaceCollisionResponse((physicsCircle*)objectB, (physicsHalfspace*)objectA);

				if (didOverlap)
				{
					objectA->color = RED;
					objectB->color = RED;
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
			physicsSimulation::physicsBody* deleteThis = *(pObjects.begin() + i);
			delete deleteThis;
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
		if (!pObjects[i]->staticBody)
		{
			pObjects[i]->position += pObjects[i]->velocity * physicsSimulationObject.deltaTime;
			pObjects[i]->velocity += physicsSimulationObject.gravity * physicsSimulationObject.deltaTime;
		}
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

	GuiSliderBar(Rectangle{ 10, 240, 500, 30 }, "Halfspace X", TextFormat("Halfspace X: %.0f", halfspace.position.x), &halfspace.position.x, 0, GetScreenWidth());

	GuiSliderBar(Rectangle{ 10, 280, 500, 30 }, "Halfspace Y", TextFormat("Halfspace Y: %.0f", halfspace.position.y), &halfspace.position.y, 0, GetScreenHeight());

	float halfspaceRotation = halfspace.getRotation();
	GuiSliderBar(Rectangle{ 10, 320, 500, 30 }, "Halfspace Rot", TextFormat("Halfspace Rot: %.0f Degrees", halfspace.getRotation()), &halfspaceRotation, -360, 360);
	halfspace.setRotation(halfspaceRotation);

	DrawText(TextFormat("Object Count: %i", pObjects.size()), GetScreenWidth() - 300, 100, 30, LIGHTGRAY);
	DrawText(TextFormat("T: %6.2f", physicsSimulationObject.time), GetScreenWidth() - 140, 10, 30, LIGHTGRAY);

	Vector2 startPos = { 100, GetScreenHeight() - 100 };
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
	halfspace.position = { 500, 700 };
	halfspace.staticBody = true;
	halfspace.setRotation(315);
	pObjects.push_back(&halfspace);

	halfspace2.position = { 400, 600 };
	halfspace2.staticBody = true;
	halfspace2.setRotation(45);
	pObjects.push_back(&halfspace2);

	while (!WindowShouldClose()) // Loops TARGET_FPS times per second
	{
		update();
		draw();
	}

	CloseWindow();
	return 0;
}