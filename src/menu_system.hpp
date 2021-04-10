#pragma once

#include "entities/screen.hpp"

class MenuSystem
{
	public:
		static void init_start_menu(ECS_ENTT::Scene* scene);
		static void init_help_menu(ECS_ENTT::Scene* scene);
		static void init_finale_menu(ECS_ENTT::Scene* scene);
		static void set_main_menu(ECS_ENTT::Scene* scene, MainMenuType type);
		static void set_help_menu(ECS_ENTT::Scene* scene, PageDirection direction);

	private:
		static unsigned int get_page_number(unsigned int page, PageDirection direction, unsigned long num_pages);
		static void remove_buttons(ECS_ENTT::Scene* scene);
};
