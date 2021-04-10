#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct HelgeProjectile
{
	static ECS_ENTT::Entity createHelgeProjectile(vec3 position, ECS_ENTT::Scene* scene);

	float timeRemaining = HELGE_PROJECTILE_LIFETIME_MS;
};