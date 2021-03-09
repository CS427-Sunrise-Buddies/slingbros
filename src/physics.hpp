#pragma once

#include "common.hpp"
#include "Entity.h"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	std::vector<std::function<void(ECS_ENTT::Entity, ECS_ENTT::Entity, bool)>> callbacks;

	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// TODO using EnTT
	// Stucture to store collision information
	struct Collision
	{
		ECS_ENTT::Entity other;

		Collision(ECS_ENTT::Entity& other);
	};

	void attach(std::function<void(ECS_ENTT::Entity, ECS_ENTT::Entity, bool)>);

	void runCollisionCallbacks(ECS_ENTT::Entity i, ECS_ENTT::Entity j, bool hit_wall);

	void getState();
};

enum VectorDir
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};
