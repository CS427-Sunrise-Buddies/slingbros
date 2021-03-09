#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct Button
{
	static ECS_ENTT::Entity createButton(vec2 position, vec2 scale, std::string buttonName, ECS_ENTT::Scene* scene);
	
	uint32_t placeholder = 0;

};
