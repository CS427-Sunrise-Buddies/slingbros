#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct Projectile
{
	// Creates all the associated render resources and default transform
	static ECS_ENTT::Entity createProjectile(vec3 position, ECS_ENTT::Scene* scene);

	float timeRemaining = BASIC_PROJECTILE_LIFETIME_MS;
};
