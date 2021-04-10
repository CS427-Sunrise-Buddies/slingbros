#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

const float BUTTON_OFFSET = 20.f;
const std::string BUTTON_NAME_NEW = "new";
const std::string BUTTON_NAME_RESUME = "resume";
const std::string BUTTON_NAME_RESUME_DISABLED = "resume_disabled";
const std::string BUTTON_NAME_QUIT = "quit";
const std::string BUTTON_NAME_QUIT_SMALL = "quit_small";
const std::string BUTTON_NAME_HELP = "help";
const std::string BUTTON_NAME_BACK = "back";
const std::string BUTTON_NAME_PREV = "prev";
const std::string BUTTON_NAME_NEXT = "next";
const std::string BUTTON_NAME_1P = "1player";
const std::string BUTTON_NAME_2P = "2player";

struct Button
{
	constexpr static const vec2 LARGE_BUTTON_SCALE = vec2(250.f, 250.f);  // Player selection buttons
	constexpr static const vec2 MEDIUM_BUTTON_SCALE = vec2(350.f, 100.f); // Buttons on start menu
	constexpr static const vec2 SMALL_BUTTON_SCALE = vec2(240.f, 80.f);   // Back button on player selection menu
	constexpr static const vec2 CIRCLE_BUTTON_SCALE = vec2(100.f, 100.f); // Help button
	constexpr static const vec2 SMALL_CIRCLE_BUTTON_SCALE = vec2(50.f, 50.f); // Prev/next buttons on help menu
	constexpr static const vec2 HELP_BUTTON_POSITION = vec2(100.f, 100.f);
	constexpr static const vec2 NEXT_BUTTON_POSITION = vec2(100.f, 100.f);
	constexpr static const vec2 PREV_BUTTON_POSITION = vec2(100.f, 100.f);

	static ECS_ENTT::Entity createButton(vec2 position, vec2 scale, const std::string& functionName, ECS_ENTT::Scene* scene);

	uint32_t placeholder = 0;
};
