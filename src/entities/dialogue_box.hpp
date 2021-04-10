#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

const glm::vec3 dialogueBoxPosition = vec3(0.f, 380.f, 0.f);
const glm::vec3 dialogueBoxScale = vec3(1150.f, 190.f, 1.0f);

struct DialogueBox
{
	static ECS_ENTT::Entity createDialogueBox(std::string fileName, ECS_ENTT::Scene* scene);
	
	uint32_t placeholder = 0;
};
