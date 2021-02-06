#pragma once

#include "common.hpp"
//#include "tiny_ecs.hpp"
#include "Entity.h"
#include "Scene.h"

struct Salmon
{
	// Creates all the associated render resources and default transform
	static ECS_ENTT::Entity createSalmon(vec2 pos, ECS_ENTT::Scene* scene);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};
