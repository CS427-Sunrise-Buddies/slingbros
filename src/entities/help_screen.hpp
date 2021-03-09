#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct HelpScreen
{
	static ECS_ENTT::Entity createHelpScreen(ECS_ENTT::Scene* scene);
	
	uint32_t placeholder = 0;

};
