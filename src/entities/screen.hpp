#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

const std::string TEXTURE_MENU = "menu_main";
const std::string TEXTURE_HELP_KEYS = "help_keys";
const std::string TEXTURE_HELP_POWERUPS = "help_powerups";

const std::map<int, std::string> HELP_TEXTURES =
		{
				{0, TEXTURE_HELP_KEYS},
				{1, TEXTURE_HELP_POWERUPS}
		};

struct Screen
{
	static ECS_ENTT::Entity createScreen(const std::string& texture_name, vec2 pos, ECS_ENTT::Scene* scene);
	
	uint32_t placeholder = 0;
};

struct Help
{
	unsigned int page = 0;
};