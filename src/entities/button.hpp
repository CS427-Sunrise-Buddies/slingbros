#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

const size_t BUTTON_OFFSET = 120;
const std::string BUTTON_NAME_NEW = "new";
const std::string BUTTON_NAME_START = "start";
const std::string BUTTON_NAME_QUIT = "quit";
const std::string BUTTON_NAME_HELP = "help";

struct Button
{
	constexpr static const vec2 MENU_BUTTON_SCALE = vec2(300.f, 100.f);
	constexpr static const vec2 HELP_BUTTON_SCALE = vec2(100.f, 100.f);
	constexpr static const vec2 HELP_BUTTON_POSITION = vec2(100.f, 100.f);

	static ECS_ENTT::Entity createButton(vec2 position, vec2 scale, const std::string& functionName, ECS_ENTT::Scene* scene);

	uint32_t placeholder = 0;
};
