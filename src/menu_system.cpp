#include <common.hpp>
#include <entities/button.hpp>
#include <loader/level_manager.hpp>
#include <entities/dialogue_box.hpp>
#include "menu_system.hpp"

void MenuSystem::init_start_menu(ECS_ENTT::Scene *scene)
{
	const glm::vec2 position = vec2(MENU_CAMERA_POSITION);
	ECS_ENTT::Entity start_screen = GameScreen::createScreen(TEXTURE_MENU, position, scene);
	start_screen.AddComponent<Main>();

	MenuSystem::set_main_menu(scene, MainMenuType::START);
}

void MenuSystem::init_help_menu(ECS_ENTT::Scene *scene)
{
	const glm::vec2 position = vec2(MENU_CAMERA_POSITION);
	ECS_ENTT::Entity help_screen = GameScreen::createScreen(HELP_TEXTURES.at(0), position, scene);
	help_screen.AddComponent<Help>();

	// Button positions
	auto y = MENU_CAMERA_POSITION.y * 2.f - Button::SMALL_CIRCLE_BUTTON_SCALE.y / 2.f - BUTTON_OFFSET; // Just a bit above bottom edge of window
	auto offset_x = Button::SMALL_CIRCLE_BUTTON_SCALE.x + BUTTON_OFFSET * 2.f;

	// Place the previous and next buttons on the screen
	Button::createButton(vec2(MENU_CAMERA_POSITION.x - offset_x, y), Button::SMALL_CIRCLE_BUTTON_SCALE, BUTTON_NAME_PREV, scene);
	Button::createButton(vec2(MENU_CAMERA_POSITION.x + offset_x, y), Button::SMALL_CIRCLE_BUTTON_SCALE, BUTTON_NAME_NEXT, scene);
}

void MenuSystem::init_finale_menu(ECS_ENTT::Scene *scene)
{
	const glm::vec2 position = vec2(MENU_CAMERA_POSITION);
	ECS_ENTT::Entity end_screen = GameScreen::createScreen(TEXTURE_FINALE, position, scene);
	end_screen.AddComponent<Finale>();

	// Button positions
	auto y = dialogueBoxScale.y + Button::SMALL_BUTTON_SCALE.y / 3.f; // Bottom edge of box in background
	auto offset_x = Button::SMALL_BUTTON_SCALE.x / 2.f + BUTTON_OFFSET;

	// Place the back and quit buttons on the screen
	Button::createButton(vec2(MENU_CAMERA_POSITION.x - offset_x, y), Button::SMALL_BUTTON_SCALE, BUTTON_NAME_BACK, scene);
	Button::createButton(vec2(MENU_CAMERA_POSITION.x + offset_x, y), Button::SMALL_BUTTON_SCALE, BUTTON_NAME_QUIT_SMALL, scene);
}

void MenuSystem::set_main_menu(ECS_ENTT::Scene *scene, MainMenuType type)
{
	// Get the current start menu page
	auto view = scene->m_Registry.view<Main>();
	assert(!view.empty() && "Scene does not have Main component -- what kinda wack stuff are you doing??");
	ECS_ENTT::Entity main_screen = ECS_ENTT::Entity(view.back(), scene);
	auto& main = main_screen.GetComponent<Main>();
	main.type = type;

	// Clear all buttons from this scene
	remove_buttons(scene);

	// Add buttons onto the screen depending on the page
	switch (main.type)
	{
		// Start menu
		case MainMenuType::START:
		{
			// Button positions
			float offset_y = Button::MEDIUM_BUTTON_SCALE.y + BUTTON_OFFSET;
			vec2 newButtonPos = vec2(MENU_CAMERA_POSITION);
			vec2 startButtonPos = vec2(MENU_CAMERA_POSITION.x, MENU_CAMERA_POSITION.y + offset_y);
			vec2 quitButtonPos = vec2(MENU_CAMERA_POSITION.x, MENU_CAMERA_POSITION.y + 2.f * offset_y);

			// Disable resume button if there is no saved file
			const std::string saved_file_path = saved_path(yaml_file(SAVE_FILE_NAME));
			auto button_name_resume = Util::file_exists(saved_file_path) ? BUTTON_NAME_RESUME : BUTTON_NAME_RESUME_DISABLED;

			// Place the help, new, resume, and quit buttons on the screen
			Button::createButton(Button::HELP_BUTTON_POSITION, Button::CIRCLE_BUTTON_SCALE, BUTTON_NAME_HELP, scene);
			Button::createButton(newButtonPos, Button::MEDIUM_BUTTON_SCALE, BUTTON_NAME_NEW, scene);
			Button::createButton(startButtonPos, Button::MEDIUM_BUTTON_SCALE, button_name_resume, scene);
			Button::createButton(quitButtonPos, Button::MEDIUM_BUTTON_SCALE, BUTTON_NAME_QUIT, scene);
			break;
		}
		// Select number of players
		case MainMenuType::PLAYER_SELECTION:
		{
			// Button positions
			auto y = MENU_CAMERA_POSITION.y + Button::LARGE_BUTTON_SCALE.y / 3.f;
			auto offset_x = Button::LARGE_BUTTON_SCALE.x / 2.f + BUTTON_OFFSET;
			auto offset_y = Button::LARGE_BUTTON_SCALE.y;

			// Place the help, 1-player, 2-player, and back buttons on the screen
			Button::createButton(Button::HELP_BUTTON_POSITION, Button::CIRCLE_BUTTON_SCALE, BUTTON_NAME_HELP, scene);
			Button::createButton(vec2(MENU_CAMERA_POSITION.x - offset_x, y), Button::LARGE_BUTTON_SCALE, BUTTON_NAME_1P, scene);
			Button::createButton(vec2(MENU_CAMERA_POSITION.x + offset_x, y), Button::LARGE_BUTTON_SCALE, BUTTON_NAME_2P, scene);
			Button::createButton(vec2(MENU_CAMERA_POSITION.x, y + offset_y), Button::SMALL_BUTTON_SCALE, BUTTON_NAME_BACK, scene);
			break;
		}
	}
}

void MenuSystem::set_help_menu(ECS_ENTT::Scene *scene, PageDirection direction)
{
	// Get the current help page
	auto view = scene->m_Registry.view<Help>();
	ECS_ENTT::Entity help_screen = ECS_ENTT::Entity(view.back(), scene);
	auto page = help_screen.GetComponent<Help>().page;

	// Get the next page texture
	unsigned int next_page = get_page_number(page, direction, HELP_TEXTURES.size());
	auto texture = HELP_TEXTURES.at(next_page);
	scene->m_Registry.destroy(help_screen);

	// Create the next page
	auto next_help_screen = GameScreen::createScreen(texture, MENU_CAMERA_POSITION, scene);
	auto& next_help = next_help_screen.AddComponent<Help>();
	next_help.page = next_page;
}

unsigned int MenuSystem::get_page_number(unsigned int page, PageDirection direction, unsigned long num_pages)
{
	// Loop back to last page
	if (page == 0 && direction == PageDirection::PREV)
	{
		return num_pages - 1;
	}

	// Modulo to cap page
	return (page + int(direction)) % num_pages;
}

void MenuSystem::remove_buttons(ECS_ENTT::Scene *scene)
{
	scene->m_Registry.view<ClickableText>().each([scene](const auto id, auto &&...) {
		scene->m_Registry.destroy(id);
	});
}
