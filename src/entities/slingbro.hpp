#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct SlingBro {
	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};

struct OrangeBro {
	static ECS_ENTT::Entity createOrangeSlingBro(vec3 position, ECS_ENTT::Scene* scene);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};

struct PinkBro {
	static ECS_ENTT::Entity createPinkSlingBro(vec3 position, ECS_ENTT::Scene* scene);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};