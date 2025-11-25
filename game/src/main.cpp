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
int ballType = 0;
float launchSpeed = 100;
float launchAngle = 0;
float gravMag = 100;
float gravDir = 270;
//float coefficientOfFriction = 0.5f;

Vector2 launchPosition = { 500, 500 };

enum physicsShape
{
	CIRCLE,
	HALFSPACE
};

class physicsSimulation
{
public:
	Vector2 gravAccel = { 0, 90 };
	float deltaTime = 1.0f / TARGET_FPS; //seconds/frame
	float time = 0.0f;
	Vector2 gravity = { launchSpeed * (float)cos(launchAngle * DEG2RAD), -launchSpeed * (float)sin(launchAngle * DEG2RAD) };
	class physicsBody
	{
	public:
		bool staticBody = false;
		float mass = 1;
		Vector2 position = { 0, 0 };
		Vector2 velocity = { 0, 0 };
		Vector2 drag = { 0, 0 };
		Color color = GREEN;
		Vector2 netForce = { 0, 0 };
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
	float coefficientOfFriction = 0.5f;
	
	physicsCircle(Vector2 conPosition, Vector2 conVelocity, float conRadius, float conCoefficientOfFriction, int conMass, Color conColor)
	{
		position = conPosition;
		velocity = conVelocity;
		radius = conRadius;
		coefficientOfFriction = conCoefficientOfFriction;
		mass = conMass;
		color = conColor;
	}
	void draw() override
	{
		DrawCircle(position.x, position.y, radius, color);
		DrawLineEx(position, position + velocity, 1, RED);
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
//physicsHalfspace halfspace2;

std::vector<physicsSimulation::physicsBody*> pObjects;

//bool circleCircleCollision(physicsCircle* circleA, physicsCircle* circleB)
//{
//	float sumRadii = circleA->radius + circleB->radius;
//	Vector2 displacement = circleB->position - circleA->position;
//
//	float distance = Vector2Length(displacement);
//
//	return (distance < sumRadii) ? true : false;
//}

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

//bool circleHalfspaceCollision(physicsCircle* circle, physicsHalfspace* halfspace)
//{
//	Vector2 displacementToCircle = circle->position - halfspace->position;
//
//	float dotProduct = Vector2DotProduct(displacementToCircle, halfspace->getNormal());
//	Vector2 vectorProjection = halfspace->getNormal() * dotProduct;
//	//float distance = Vector2Length(displacementToCircle);
//
//	DrawLineEx(circle->position, circle->position - vectorProjection, 1, GRAY);
//
//	Vector2 midpoint = circle->position - vectorProjection * 0.5f;
//	DrawText(TextFormat("D: %3.0f", dotProduct), midpoint.x, midpoint.y, 30, LIGHTGRAY);
//
//	return dotProduct < circle->radius;
//}

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
		Vector2 Fgravity = physicsSimulationObject.gravAccel * circle->mass;

		Vector2 FgPerp = halfspace->getNormal() * Vector2DotProduct(Fgravity, halfspace->getNormal());
		Vector2 Fnormal = FgPerp * -1;
		circle->netForce += Fnormal;
		DrawLineEx(circle->position, circle->position + Fnormal, 1, GREEN);

		float u = circle->coefficientOfFriction;
		float frictionMagnitude = u * Vector2Length(Fnormal);

		Vector2 FgPara = Fgravity - FgPerp;
		Vector2 frictionDir = Vector2Normalize(FgPara) * -1;

		Vector2 Ffriction = frictionDir * frictionMagnitude;

		circle->netForce += Ffriction;
		DrawLineEx(circle->position, circle->position + Ffriction, 1, ORANGE);

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
		//pObjects[i]->color = GREEN;
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
					//objectA->color = RED;
					//objectB->color = RED;
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

void resetNetForces()
{
	for (int i = 0; i < pObjects.size(); i++)
	{
		pObjects[i]->netForce = { 0, 0 };
	}
}

void addGravForces()
{
	
	//physicsSimulationObject.gravity = { gravMag * (float)cos(gravDir * DEG2RAD), -gravMag * (float)sin(gravDir * DEG2RAD) };
	for (int i = 0; i < pObjects.size(); i++)
	{
		if (!pObjects[i]->staticBody)
		{

			//pObjects[i]->position += pObjects[i]->velocity * physicsSimulationObject.deltaTime;
			//pObjects[i]->velocity += physicsSimulationObject.gravity * physicsSimulationObject.deltaTime;
			Vector2 FGravity = physicsSimulationObject.gravAccel * pObjects[i]->mass;
			pObjects[i]->netForce += FGravity;
		}
	}
}

void applyKinematics()
{
	for (int i = 0; i < pObjects.size(); i++)
	{
		if (!pObjects[i]->staticBody)
		{

			//pObjects[i]->position += pObjects[i]->velocity * physicsSimulationObject.deltaTime;
			//pObjects[i]->velocity += physicsSimulationObject.gravity * physicsSimulationObject.deltaTime;
			//Vector2 FGravity = physicsSimulationObject.gravAccel * pObjects[i]->mass;
			//pObjects[i]->netForce += FGravity;
			pObjects[i]->position += pObjects[i]->velocity * physicsSimulationObject.deltaTime;

			Vector2 acceleration = pObjects[i]->netForce / pObjects[i]->mass;
			
			pObjects[i]->velocity += acceleration * physicsSimulationObject.deltaTime;
			//DrawLineEx(pObjects[i]->position, pObjects[i]->position + pObjects[i]->netForce, 1, GRAY);
			DrawLineEx(pObjects[i]->position, pObjects[i]->position + (physicsSimulationObject.gravAccel * pObjects[i]->mass), 1, PURPLE);
		}
	}
}

//Changes world state
void update()
{
	physicsSimulationObject.time += physicsSimulationObject.deltaTime;
	//vel = change in position / time, therefore change in position = vel * time
	resetNetForces();
	addGravForces();
	collision();
	applyKinematics();

	//accel = deltaV / time (change in velocity over time) therefore deltaV = accel * time
	

	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        launchPosition = GetMousePosition();

	if (IsKeyPressed(KEY_SPACE))
	{
		Vector2 velocity = {launchSpeed * (float)cos(launchAngle * DEG2RAD), -launchSpeed * (float)sin(launchAngle * DEG2RAD)};
		float newRadius = 15;
		Color newColor;
		int newMass;
		float newFric;
		switch (ballType)
		{
		case 0:
			newColor = RED;
			newMass = 2;
			newFric = 0.1f;
			ballType++;
			break;
		case 1:
			newColor = GREEN;
			newMass = 2;
			newFric = 0.8f;
			ballType++;
			break;
		case 2:
			newColor = BLUE;
			newMass = 8;
			newFric = 0.1f;
			ballType++;
			break;
		case 3:
			newColor = YELLOW;
			newMass = 8;
			newFric = 0.8f;
			ballType = 0;
			break;
		}
		physicsCircle* newCircle = new physicsCircle(launchPosition, velocity, newRadius, newFric, newMass, newColor);
		pObjects.push_back(newCircle);
	}
	
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

	//GuiSliderBar(Rectangle{ 10, 160, 500, 30 }, "Gravity Magnitude", TextFormat("Magnitude: %.0f", gravMag), &gravMag, -1000, 1000);

	//GuiSliderBar(Rectangle{ 10, 200, 500, 30 }, "Gravity Direction", TextFormat("Direction: %.0f Degrees", gravDir), &gravDir, 0, 360);

	GuiSliderBar(Rectangle{ 10, 160, 500, 30 }, "Gravity Magnitude", TextFormat("Magnitude: %.0f", physicsSimulationObject.gravAccel.y), &physicsSimulationObject.gravAccel.y, -1000, 1000);

	GuiSliderBar(Rectangle{ 10, 240, 500, 30 }, "Halfspace X", TextFormat("Halfspace X: %.0f", halfspace.position.x), &halfspace.position.x, 0, GetScreenWidth());

	GuiSliderBar(Rectangle{ 10, 280, 500, 30 }, "Halfspace Y", TextFormat("Halfspace Y: %.0f", halfspace.position.y), &halfspace.position.y, 0, GetScreenHeight());

	float halfspaceRotation = halfspace.getRotation();
	GuiSliderBar(Rectangle{ 10, 320, 500, 30 }, "Halfspace Rot", TextFormat("Halfspace Rot: %.0f Degrees", halfspace.getRotation()), &halfspaceRotation, -360, 360);
	halfspace.setRotation(halfspaceRotation);

	DrawText(TextFormat("Object Count: %i", pObjects.size()), GetScreenWidth() - 300, 100, 30, LIGHTGRAY);
	DrawText(TextFormat("T: %6.2f", physicsSimulationObject.time), GetScreenWidth() - 140, 10, 30, LIGHTGRAY);

	//Vector2 startPos = { 100, GetScreenHeight() - 100 };
	Vector2 velocity = { launchSpeed * cos(launchAngle * DEG2RAD), -launchSpeed * sin(launchAngle * DEG2RAD)};

	DrawLineEx(launchPosition, launchPosition + velocity, 3, RED);
	for (int i = 0; i < pObjects.size(); i++)
	{
		/*float mass = 1;
		Vector2 Fgravity = physicsSimulationObject.gravity * mass;
		DrawLine(pObjects[i]->position.x, pObjects[i]->position.y, pObjects[i]->position.x + Fgravity.x, pObjects[i]->position.y + Fgravity.y, PURPLE);
		
		

		Vector2 FgPara = Fgravity - FgPerp;
		Vector2 Ffriction = FgPara * -1;
		DrawLine(pObjects[i]->position.x, pObjects[i]->position.y, pObjects[i]->position.x + Ffriction.x, pObjects[i]->position.y + Ffriction.y, ORANGE);
		
		pObjects[i]->velocity += Ffriction;*/

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

	/*halfspace2.position = {400, 600};
	halfspace2.staticBody = true;
	halfspace2.setRotation(45);
	pObjects.push_back(&halfspace2);*/

	while (!WindowShouldClose()) // Loops TARGET_FPS times per second
	{
		update();
		draw();
	}

	CloseWindow();
	return 0;
}