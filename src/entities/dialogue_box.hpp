#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct DialogueBox
{
	static ECS_ENTT::Entity createDialogueBox(std::string fileName, ECS_ENTT::Scene* scene);
	
	uint32_t placeholder = 0;

};
