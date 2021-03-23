#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct ParallaxBackground {

	static ECS_ENTT::Entity createBackground(ECS_ENTT::Scene* scene, std::string backgroundFilename);

	static uint32_t backgroundID;

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};