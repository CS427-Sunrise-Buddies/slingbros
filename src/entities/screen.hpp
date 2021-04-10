#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

// Main menu
enum class MainMenuType
{
	START = 0,
	PLAYER_SELECTION = 1,
};
const std::string TEXTURE_MENU = "menu_main";

// Help menu
enum class PageDirection
{
	PREV = -1,
	NEXT = 1,
};
const std::string TEXTURE_HELP_KEYS = "help_keys";
const std::string TEXTURE_HELP_POWERUPS = "help_powerups";
const std::string TEXTURE_HELP_TILES = "help_tiles";
const std::string TEXTURE_HELP_WEATHER = "help_weather";
const std::vector HELP_TEXTURES =
		{
				TEXTURE_HELP_KEYS,
				TEXTURE_HELP_POWERUPS,
				TEXTURE_HELP_TILES,
				TEXTURE_HELP_WEATHER,
		};

// Finale menu
const std::string TEXTURE_FINALE = "menu_finale";

struct GameScreen
{
	static ECS_ENTT::Entity createScreen(const std::string& texture_name, vec2 pos, ECS_ENTT::Scene* scene);
	
	uint32_t placeholder = 0;
};

struct Main
{
	MainMenuType type = MainMenuType::START;
};

struct Help
{
	unsigned int page = 0;
};

struct Finale
{
	unsigned int page = 0; // unused, placeholder
};