#define _USE_MATH_DEFINES
#include <cmath>

// Header
#include "world.hpp"
#include "physics.hpp"
#include "debug.hpp"
#include "render_components.hpp"
#include "animation.hpp" 
#include "loader/level_manager.hpp"
#include "text.hpp"

#include <glm/ext/matrix_transform.hpp>

// stlib
#include <string.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>
#include <entities/windy_grass.hpp>
#include <entities/grassy_tile.hpp>
#include <entities/lava_tile.hpp>
#include <entities/screen.hpp>

// Entities
#include "entities/powerup.hpp"
#include "entities/speed_powerup.hpp"
#include "entities/slingbro.hpp"
#include "entities/projectile.hpp"
#include "entities/button.hpp"
#include "entities/goal_tile.hpp"
#include "entities/snail_enemy.hpp"
#include "entities/start_tile.hpp"
#include "entities/size_up_powerup.hpp"
#include "entities/size_down_powerup.hpp"
#include "entities/dialogue_box.hpp"

// Game configuration
ECS_ENTT::Scene* WorldSystem::ActiveScene = nullptr;
std::stack<ECS_ENTT::Scene*> WorldSystem::PrevScenes;

// For this barebones template, this is just the main scene (Scenes are essentially just entity containers)
ECS_ENTT::Scene* WorldSystem::GameScene = nullptr;
ECS_ENTT::Scene* WorldSystem::MenuScene = nullptr;
ECS_ENTT::Scene* WorldSystem::HelpScene = nullptr;


const unsigned int WorldSystem::num_players = MAX_NUM_PLAYERS;
bool WorldSystem::is_ai_turn = true;

bool isLoadNextLevel = false;

typedef ECS_ENTT::Entity (*fn)(vec3, ECS_ENTT::Scene*);
typedef std::map<int, fn> SlingBroFunctionMap;
const SlingBroFunctionMap slingBroFunctions =
		{
				{0, OrangeBro::createOrangeSlingBro},
				{1, PinkBro::createPinkSlingBro}
		};

// Create the slingBro world
// Note, this has a lot of OpenGL specific things, could be moved to the renderer; but it also defines the callbacks to the mouse and keyboard. That is why it is called here.
WorldSystem::WorldSystem(ivec2 window_size_px) :
		levels(LevelManager::get_levels())
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());

	///////////////////////////////////////
	// Initialize GLFW
	auto glfw_err_callback = [](int error, const char* desc)
	{ std::cerr << "OpenGL:" << error << desc << std::endl; };
	glfwSetErrorCallback(glfw_err_callback);
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	//-------------------------------------------------------------------------
	// GLFW / OGL Initialization, needs to be set before glfwCreateWindow
	// Core Opengl 3.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_size_px.x, window_size_px.y, "Slingbro", nullptr, nullptr);
	if (window == nullptr)
		throw std::runtime_error("Failed to glfwCreateWindow");

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3)
	{
		((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3);
	};
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1)
	{
		((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 });
	};
	auto cursor_click_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2)
	{
		((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2);
	};
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, cursor_click_redirect);

	// Playing background music indefinitely
	init_audio();
	Mix_PlayMusic(background_music, -1);
	std::cout << "Loaded music\n";

	WorldSystem::MenuScene = new ECS_ENTT::Scene("Menu", { 10, 10 });
	// Initialize menu camera properties
	Camera* menuCamera = WorldSystem::MenuScene->GetCamera();
	menuCamera->SetViewportSize(WINDOW_SIZE_IN_PX.x, WINDOW_SIZE_IN_PX.y);
	menuCamera->SetPosition(MENU_CAMERA_POSITION);
	menuCamera->SetPerspective(glm::radians(60.0f), 0.01f, MENU_CAMERA_PERSPECTIVE_FAR_BOUND);
	menuCamera->SetFixed(true);

	WorldSystem::GameScene = new ECS_ENTT::Scene("Default", { 10, 10 });
	// Initialize game camera properties
	Camera* gameSceneCamera = WorldSystem::GameScene->GetCamera();
	gameSceneCamera->SetViewportSize(WINDOW_SIZE_IN_PX.x, WINDOW_SIZE_IN_PX.y);
	gameSceneCamera->SetPosition(glm::vec3(400, 300, 600));
	gameSceneCamera->SetPerspective(glm::radians(80.0f), 0.01f, GAME_CAMERA_PERSPECTIVE_FAR_BOUND);

	WorldSystem::HelpScene = new ECS_ENTT::Scene("Help", {10, 10});
	// Initialize help camera properties
	Camera* helpCamera = WorldSystem::HelpScene->GetCamera();
	helpCamera->SetViewportSize(WINDOW_SIZE_IN_PX.x, WINDOW_SIZE_IN_PX.y);
	helpCamera->SetPosition(MENU_CAMERA_POSITION);
	helpCamera->SetPerspective(glm::radians(60.0f), 0.01f, MENU_CAMERA_PERSPECTIVE_FAR_BOUND);
	helpCamera->SetFixed(true);
	
	// Initialize Game on the Menu scene
	WorldSystem::ActiveScene = MenuScene;
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);
	if (yeehaw_sound != nullptr)
		Mix_FreeChunk(yeehaw_sound);
	if (poppin_click_sound != nullptr)
		Mix_FreeChunk(poppin_click_sound);
	if (transport_sound != nullptr)
		Mix_FreeChunk(transport_sound);
	if (short_grass_sound != nullptr)
		Mix_FreeChunk(short_grass_sound);
	if (shorter_grass_sound != nullptr)
		Mix_FreeChunk(shorter_grass_sound);
	if (short_thud_sound != nullptr)
		Mix_FreeChunk(short_thud_sound);
	if (power_up_sound != nullptr)
		Mix_FreeChunk(power_up_sound);
	if (sizzle_sound != nullptr)
		Mix_FreeChunk(sizzle_sound);
	if (power_up_8bit_sound != nullptr)
		Mix_FreeChunk(power_up_8bit_sound);
	if (balloon_tap_sound != nullptr)
		Mix_FreeChunk(balloon_tap_sound);
	if (snail_monster_sound != nullptr)
		Mix_FreeChunk(snail_monster_sound);
	if (squeak_sound != nullptr)
		Mix_FreeChunk(squeak_sound);
	if (short_monster_sound != nullptr)
		Mix_FreeChunk(short_monster_sound);
	Mix_CloseAudio();

	// TODO using EnTT
	// Destroy all created components
	//ECS::ContainerInterface::clear_all_components();

	// Free memory of all of the game's scenes
	delete (GameScene);
	delete (HelpScene);
	delete (MenuScene);

	// Close the window
	glfwDestroyWindow(window);
}

void WorldSystem::init_audio()
{
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
		throw std::runtime_error("Failed to initialize SDL Audio");

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
		throw std::runtime_error("Failed to open audio device");

	salmon_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("salmon_eat.wav").c_str());

	background_music = Mix_LoadMUS(audio_path("background_music.wav").c_str());
	yeehaw_sound = Mix_LoadWAV(audio_path("yeehaw.wav").c_str());
	poppin_click_sound = Mix_LoadWAV(audio_path("pop.wav").c_str());
	transport_sound = Mix_LoadWAV(audio_path("long_magic.wav").c_str());
	short_grass_sound = Mix_LoadWAV(audio_path("short_grass.wav").c_str());
	shorter_grass_sound = Mix_LoadWAV(audio_path("shorter_grass.wav").c_str());
	short_thud_sound = Mix_LoadWAV(audio_path("slap.wav").c_str());
	power_up_sound = Mix_LoadWAV(audio_path("power_up.wav").c_str());
	sizzle_sound = Mix_LoadWAV(audio_path("sizzle.wav").c_str());
	power_up_8bit_sound = Mix_LoadWAV(audio_path("power_up_8bit.wav").c_str());
	balloon_tap_sound = Mix_LoadWAV(audio_path("balloon_tap.wav").c_str());
	snail_monster_sound = Mix_LoadWAV(audio_path("snail_monster.wav").c_str());
	squeak_sound = Mix_LoadWAV(audio_path("squeak.wav").c_str());
	short_monster_sound = Mix_LoadWAV(audio_path("short_monster.wav").c_str());


//	if (salmon_dead_sound == nullptr || salmon_eat_sound == nullptr ||
//		background_music == nullptr || happy_noises_sound == nullptr || poppin_click_sound == nullptr ||
//		transport_sound == nullptr)
//		throw std::runtime_error("Failed to load sounds make sure the data directory is present: " +
//								 audio_path("music.wav") +
//								 audio_path("salmon_dead.wav") +
//								 audio_path("salmon_eat.wav"));
}


ECS_ENTT::Scene* WorldSystem::MenuInit(vec2 window_size_in_game_units)
{
	ECS_ENTT::Scene* menuScene = WorldSystem::MenuScene;
	const glm::vec2 screenPosition = vec2(MENU_CAMERA_POSITION);
	Screen::createScreen(TEXTURE_MENU, screenPosition, menuScene);

	// todo(atsang): implement new game logic
	//vec2 newButtonPos = vec2(MENU_CAMERA_POSITION);
	vec2 startButtonPos = vec2(MENU_CAMERA_POSITION);
	vec2 quitButtonPos = vec2(MENU_CAMERA_POSITION.x, MENU_CAMERA_POSITION.y + BUTTON_OFFSET);

	//Button::createButton(newButtonPos, Button::MENU_BUTTON_SCALE, BUTTON_NAME_NEW, menuScene);
	Button::createButton(startButtonPos, Button::MENU_BUTTON_SCALE, BUTTON_NAME_START, menuScene);
	Button::createButton(quitButtonPos,  Button::MENU_BUTTON_SCALE, BUTTON_NAME_QUIT, menuScene);
	Button::createButton(Button::HELP_BUTTON_POSITION,  Button::HELP_BUTTON_SCALE, BUTTON_NAME_HELP, menuScene);

	return menuScene;
}

ECS_ENTT::Scene* WorldSystem::HelpInit()
{
	ECS_ENTT::Scene* helpScene = WorldSystem::HelpScene;
	const glm::vec2 helpEntityPosition = MENU_CAMERA_POSITION;
	ECS_ENTT::Entity helpScreenEntity = Screen::createScreen(HELP_TEXTURES.at(0), helpEntityPosition, helpScene);
	helpScreenEntity.AddComponent<Help>();

	return helpScene;
}

// Update our game world
ECS_ENTT::Scene* WorldSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << ActiveScene->m_Name;
	if (is_game_scene())
	{
		if (is_ai_turn) {
			title_ss << " | Turn: AI";
		} else {
			auto turn = get_current_player().GetComponent<Turn>();
			auto countdown = ceil(turn.countdown / 100.f) / 10.f;
			title_ss << " | Turn: Player " << ActiveScene->GetPlayer() << " (end turn in " << countdown << "s)" << " | Points: " << turn.points;
		}
	}
	glfwSetWindowTitle(window, title_ss.str().c_str());

	
	if (ActiveScene->GetDialogueBoxNames().size() > 0 && !ActiveScene->is_in_dialogue) {
		ActiveScene->is_in_dialogue = true;
		handleDialogue();
	}
	
	// End player turn
	if (is_game_scene() && should_end_turn(elapsed_ms))
	{
		current_player_effects_tick();
		Mix_PlayChannel(-1, squeak_sound, 0);

		set_next_player();
	}

	if (is_moving(get_current_player())) {
		point_camera_at_current_player();
	}

	// TODO Removing out of screen entities (for the appropriate entities like projectiles, for example)

	return ActiveScene;
}


void WorldSystem::handleDialogue() {
	// TODO disable rotation
	get_current_player().GetComponent<SlingMotion>().canClick = false;
	ActiveScene->GetCamera()->SetFixed(true);
	
	// Remove previous box
	for (auto entity : ActiveScene->m_Registry.view<DialogueBox>()) {
		ActiveScene->m_Registry.destroy(entity);
	}
	
	std::queue<std::string> dialogue_boxes = ActiveScene->GetDialogueBoxNames();
	if (dialogue_boxes.size() > 0 && ActiveScene->is_in_dialogue) {
		// Create new box
		std::string current_box = dialogue_boxes.front();
		ActiveScene->PopDialogueBoxNames();
		DialogueBox::createDialogueBox(current_box, ActiveScene);
	} else {
		ActiveScene->is_in_dialogue = false;
		for (auto entity : ActiveScene->m_Registry.view<DialogueBox>()) {
			ActiveScene->m_Registry.destroy(entity);
		}
		ActiveScene->current_dialogue_box = "";
		get_current_player().GetComponent<SlingMotion>().canClick = true;
		ActiveScene->GetCamera()->SetFixed(false);
	}
}

void WorldSystem::current_player_effects_tick() {
	auto current_player = get_current_player();

	if (current_player.HasComponent<SizeChanged>()) {
		auto& sizeChangedComponent = current_player.GetComponent<SizeChanged>();

		if (sizeChangedComponent.turnsRemaining <= 0) {
			current_player.RemoveComponent<SizeChanged>();
			auto& motionComponent = current_player.GetComponent<Motion>();
			motionComponent.scale = { SPRITE_SCALE, SPRITE_SCALE, 1.0f };
		}
		else {
			sizeChangedComponent.turnsRemaining -= 1;
		}
	}
}

void WorldSystem::remove_all_entities() {
	GameScene->m_Registry.each([](const auto entityID, auto &&...) {
		GameScene->m_Registry.destroy(entityID);
	});
}

template<typename ComponentType>
void WorldSystem::RemoveAllEntitiesWithComponent()
{
	ActiveScene->m_Registry.view<ComponentType>().each([](const auto entityID, auto &&...) {
		ActiveScene->m_Registry.destroy(entityID);
	});
}

bool WorldSystem::getIsLoadNextLevel() {
	return isLoadNextLevel;
}

void WorldSystem::setIsLoadNextLevel(bool b) {
	isLoadNextLevel = b;
}

void WorldSystem::load_next_level()
{
	unsigned int levelNum = get_level_number() + 1;
	if (levelNum == levels.size()) {
		return;
	}

	std::cout << "loading next level\n";

	int turnPoints[num_players];
	reward_current_player();
	save_turn_point_information(turnPoints);

	unsigned int next_player_idx = set_next_player();
	// Remove all old entities in the scene
	remove_all_entities();

	// Load the next level
	const std::string level_file_path = levels_path(yaml_file(levels[levelNum]));
	load_level(level_file_path, true);
	ActiveScene = GameScene;
	ActiveScene->SetPlayer(next_player_idx);

	// Current player reached the goal tile to get
	// to this level so increment to the next player
	load_turn_point_information(turnPoints);
}

// Reset the world state to its initial state
void WorldSystem::restart() // notes: like Game::init
{
	std::cout << "Restarting\n";

	ParticleSystem::GetInstance()->clearParticles();

	// Remove all old entities in the scene
	remove_all_entities();

	// todo(atsang): Remove in M4 submission
	//  Example usage of create_level as a dev tool
	//  1. Clone data/create/tutorial.yaml to make a new map
	//  2. Call create_level on the new file to generate the level yaml file
	//  3. Run the game
	//  4. Copy over the generated level yaml file from the build directory to the repo
	//  5. Add the file name (minus the .yaml extension) to data/config/config.yaml in whatever order
	// LevelManager::create_level("YourNewLevel.yaml")
	// Uncomment below to regenerate all levels
    // for (const auto& level_name : levels) LevelManager::create_level(yaml_file(level_name));

	// Load the scene
	WorldSystem::reload_level();
}

void WorldSystem::powerup_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, ECS_ENTT::Scene* gameScene) {
	Mix_PlayChannel(-1, power_up_sound, 0);

	if (entity_j.HasComponent<SpeedPowerUp>()) {
		SpeedPowerUp& speed_power_up = entity_j.GetComponent<SpeedPowerUp>();
		speed_power_up.applyPowerUp(entity_i);
	}

	if (entity_j.HasComponent<SizeUpPowerUp>()) {
		SizeUpPowerUp& size_up_power_up = entity_j.GetComponent<SizeUpPowerUp>();
		size_up_power_up.applyPowerUp(entity_i);
	}

	if (entity_j.HasComponent<SizeDownPowerUp>()) {
		SizeDownPowerUp& size_down_power_up = entity_j.GetComponent<SizeDownPowerUp>();
		size_down_power_up.applyPowerUp(entity_i);
	}

	gameScene->m_Registry.destroy(entity_j);
}

// Collisions between wall and non-wall entities - callback function, listening to PhysicsSystem::Collisions
// example of observer pattern
void WorldSystem::collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall)
{
	if (entity_i.HasComponent<SlingBro>() && DebugSystem::in_debug_mode) {
		DebugSystem::in_freeze_mode = true;
	}

	auto current_player = get_current_player();

	if (!hit_wall && entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<Projectile>()) {
		auto& turn = entity_i.GetComponent<Turn>();
		--turn.points;
		ActiveScene->m_Registry.destroy(entity_j);
	} else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<PowerUp>()) {
		powerup_collision_listener(entity_i, entity_j, ActiveScene);
	}
	else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<GoalTile>())
	{
		Mix_PlayChannel(-1, transport_sound, 0);
		Mix_PlayChannel(-1, yeehaw_sound, 0);
		setIsLoadNextLevel(true);
	}
	else if (entity_i.HasComponent<SnailEnemy>() && entity_j.HasComponent<GoalTile>())
	{
		setIsLoadNextLevel(true);
	}
	else if (entity_j.HasComponent<PowerUp>())
	{
		Mix_PlayChannel(-1, power_up_sound, 0);
	}
	else if (is_moving(current_player) && entity_i == current_player)
	{
		// slingBro sound effects for tiles
		if (entity_i.HasComponent<SlingBro>())
		{
			if (entity_j.HasComponent<WindyGrass>())
			{
				Mix_PlayChannel(-1, short_grass_sound, 0);
			}
			else if (entity_j.HasComponent<GrassyTile>())
			{
				Mix_PlayChannel(-1, shorter_grass_sound, 0);
			}
			else if (entity_j.HasComponent<LavaTile>())
			{
				Mix_PlayChannel(-1, sizzle_sound, 0);

			}
			else if (hit_wall)
			{
				Mix_PlayChannel(-1, short_thud_sound, 0);

			}
		}
	}
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return glfwWindowShouldClose(window) > 0;
}

bool WorldSystem::IsKeyPressed(const int glfwKeycode)
{
	auto keyState = glfwGetKey(window, static_cast<int32_t>(glfwKeycode));
	
	return (keyState == GLFW_PRESS || keyState == GLFW_REPEAT);
}

// On key callback
// See: https://www.glfw.org/docs/3.3/input_guide.html
void WorldSystem::on_key(int key, int, int action, int mod)
{	
	if (is_menu_scene())
	{
		if (action == GLFW_RELEASE && key == GLFW_KEY_H)
		{
			PrevScenes.push(ActiveScene);
			ActiveScene = HelpScene;
		}
		
		if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
		{
			exit(0);
		}
	}
	else if (is_help_scene())
	{
		// Flip between pages
		if (action == GLFW_RELEASE && key == GLFW_KEY_H)
		{
			// Get the current help page
			auto view = WorldSystem::HelpScene->m_Registry.view<Help>();
			ECS_ENTT::Entity help_screen = ECS_ENTT::Entity(view.back(), WorldSystem::HelpScene);
			auto page = help_screen.GetComponent<Help>().page;

			// Get the next page texture
			unsigned int next_page = (page + 1) % HELP_TEXTURES.size();
			auto texture = HELP_TEXTURES.at(next_page);
			HelpScene->m_Registry.destroy(help_screen);

			// Create the next page
			auto next_help_screen = Screen::createScreen(texture, MENU_CAMERA_POSITION, HelpScene);
			auto& next_help = next_help_screen.AddComponent<Help>();
			next_help.page = next_page;
		}

		if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
		{
			ECS_ENTT::Scene* tempScene = PrevScenes.top();
			PrevScenes.pop();
			PrevScenes.push(ActiveScene);
			ActiveScene = tempScene;
		}
	}
	// A game scene is active
	else if (ActiveScene == GameScene && ActiveScene->is_in_dialogue)
	{
		// Resetting game
		if (action == GLFW_RELEASE) {
			if (key == GLFW_KEY_R)
			{
				restart();
			}
			// Bring up help scene
			else if (key == GLFW_KEY_H)
			{
				PrevScenes.push(ActiveScene);
				ActiveScene = HelpScene;
			}
			else if (key == GLFW_KEY_ESCAPE)
			{
				PrevScenes.push(ActiveScene);
				ActiveScene = MenuScene;
			}
			// Saving
			else if (key == GLFW_KEY_ENTER)
			{
				LevelManager::save_level(GameScene);
			}
			else
			{
				handleDialogue();
			}
		}
	}
	else
	{
		// Restart current level
		if (action == GLFW_RELEASE)
		{
			if (key == GLFW_KEY_R)
			{
				restart();
			}
			
			// Return to main menu
			if (key == GLFW_KEY_ESCAPE)
			{
				PrevScenes.push(ActiveScene);
				ActiveScene = MenuScene;
			}
			
			// Bring up help scene
			if (key == GLFW_KEY_H)
			{
				PrevScenes.push(ActiveScene);
				ActiveScene = HelpScene;
			}
			
			// Saving
			if (key == GLFW_KEY_ENTER)
			{
				LevelManager::save_level(GameScene);
			}
		}

		// Debugging
		if (key == GLFW_KEY_Z)
		{
			DebugSystem::in_debug_mode = (action != GLFW_RELEASE);
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_pos)
{
	auto slingBro = get_current_player();
	if (slingBro.GetComponent<SlingMotion>().isClicked)
	{
		Mix_PlayChannel(-1, balloon_tap_sound, 0);

	}
	if (is_menu_scene())
	{
		for (auto entity : WorldSystem::ActiveScene->m_Registry.view<ClickableText>())
		{
			Motion textComponent = WorldSystem::ActiveScene->m_Registry.get<Motion>(entity);
			//WorldSystem::ActiveScene->m_Registry.get<Text>(entity);
			// TODO: can't invert y properly until scaling is fixed

			glm::vec2 dispVecFromText = WorldSystem::getDispVecFromSource(glm::vec2(mouse_pos), glm::vec3(textComponent.position));

			float scaleX = abs(textComponent.scale.x / 2.f);
			float scaleY = abs(textComponent.scale.y / 2.f);

			ClickableText& clickableText = WorldSystem::ActiveScene->m_Registry.get<ClickableText>(entity);
			clickableText.isHoveredOver = dispVecFromText.x >= -scaleX && dispVecFromText.x <= scaleX && dispVecFromText.y >= -scaleY && dispVecFromText.y <= scaleY && clickableText.canClick;
		}
	}

	if (is_game_scene() && !is_ai_turn && !ActiveScene->is_in_dialogue)
	{
		auto slingbro = get_current_player();

		if (!slingbro.HasComponent<DeathTimer>())
		{
			auto& m_slingbro = slingbro.GetComponent<Motion>();
			glm::vec2 dispVecFromBro = getDispVecFromSource(glm::vec2(mouse_pos), glm::vec3(m_slingbro.position));
			float angle = atan2(dispVecFromBro.y, dispVecFromBro.x);
			// Offset angle by pi/2 to correct rotation angle
			m_slingbro.angle = angle + M_PI_2;
		}
	}
	
	// TODO:
	/*
	- find bounding box/calculate centre of character
	 **/
}

void WorldSystem::on_mouse_click(int button, int action, int mods)
{
	// Get mouse click position
	double mouseXPos, mouseYPos;
	glfwGetCursorPos(window, &mouseXPos, &mouseYPos);
	vec2 mouse_pos = vec2(mouseXPos, mouseYPos);

	if (is_game_scene())
	{
		// Advance dialog
		if (action == GLFW_PRESS && ActiveScene->is_in_dialogue)
		{
			handleDialogue();
		}
		else
		{
			auto slingbro = get_current_player();
			auto& broMotion = slingbro.GetComponent<Motion>();

			// Convert character's position from local/world coords to screen coords
			glm::vec2 dispVecFromBro = WorldSystem::getDispVecFromSource(glm::vec2(mouse_pos), glm::vec3(broMotion.position));

			auto& broSlingMotion = slingbro.GetComponent<SlingMotion>();
			bool& canClick = broSlingMotion.canClick;
			bool& isBroClicked = broSlingMotion.isClicked;
			auto& clickPosition = broSlingMotion.clickPosition;
			vec2& dragDir = broSlingMotion.direction;
			vec2 dragMagnitude = broSlingMotion.magnitude;
			vec3& broVelocity = broMotion.velocity;

			// Start sling
			if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			{
				// Keep track of the mouse position of the click
				clickPosition = mouse_pos;

				if (abs(dispVecFromBro.x) < broMotion.scale.x && abs(dispVecFromBro.y) < broMotion.scale.y && canClick)
				{
					isBroClicked = !WorldSystem::is_ai_turn;
				}
			}

			// End sling
			else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && isBroClicked)
			{
				// Already had a turn
				auto& turn = slingbro.GetComponent<Turn>();
				if (turn.slung)
				{
					return;
				}

				// Don't count clicks with no drag
				if (length(mouse_pos - clickPosition) < MIN_DRAG_LENGTH)
				{
					return;
				}

				isBroClicked = false;
				dragDir = -dispVecFromBro;
				dragMagnitude *= length(dragDir);
				dragDir.x *= dragMagnitude.x;
				dragDir.y *= dragMagnitude.y;
				broVelocity = vec3(dragDir, 0.0) / 1000.f;

				// setting max speed for the bro, maybe load the velocities when we have the level loader
				broVelocity.x = glm::clamp(broVelocity.x, -MAX_VELOCITY, MAX_VELOCITY);
				broVelocity.y = glm::clamp(broVelocity.y, -MAX_VELOCITY, MAX_VELOCITY);

				// Record that player has slung
				turn.slung = true;
			}
		}
	}

	if (is_menu_scene())
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		{
			for (auto entity : WorldSystem::ActiveScene->m_Registry.view<ClickableText>())
			{
				ClickableText& clickableText = ActiveScene->m_Registry.get<ClickableText>(entity);
				if (clickableText.isHoveredOver && !clickableText.isClicked)
				{
					clickableText.isClicked = true;
					Mix_PlayChannel(-1, poppin_click_sound, 0);
					if (clickableText.functionName == BUTTON_NAME_START)
					{
						printf("Start game\n");
						clickableText.isClicked = false;
						ActiveScene = GameScene;
						point_camera_at_current_player();
					}
					else if (clickableText.functionName == BUTTON_NAME_QUIT)
					{
						printf("Exit game\n");
						clickableText.isClicked = false;
						exit(0);
					}
					else if (clickableText.functionName == BUTTON_NAME_HELP)
					{
						printf("Open help menu\n");
						clickableText.isClicked = false;
						PrevScenes.push(ActiveScene);
						ActiveScene = HelpScene;
					}
					// Click only one button at a time
					break;
				}
			}
		}
	}
}

void WorldSystem::HandleCameraMovement(Camera* camera, float deltaTime)
{
	if (camera->IsFixed())
		return;

	glm::vec3 cameraPosition = camera->GetPosition();
	float cameraSpeed = 80.0f;

	// WASD keys
	if (IsKeyPressed(GLFW_KEY_W))
		cameraPosition.y -= cameraSpeed * deltaTime / 100.0f;
	if (IsKeyPressed(GLFW_KEY_S))
		cameraPosition.y += cameraSpeed * deltaTime / 100.0f;
	if (IsKeyPressed(GLFW_KEY_A))
		cameraPosition.x -= cameraSpeed * deltaTime / 100.0f;
	if (IsKeyPressed(GLFW_KEY_D))
		cameraPosition.x += cameraSpeed * deltaTime / 100.0f;

	// Q and E keys to control camera's Z rotation
	float cameraRotationZ = camera->GetRotationZ();
	float cameraRotationSpeed = 4.2f;
	if (IsKeyPressed(GLFW_KEY_Q))
		cameraRotationZ -= cameraRotationSpeed * deltaTime / 100.0f;
	if (IsKeyPressed(GLFW_KEY_E))
		cameraRotationZ += cameraRotationSpeed * deltaTime / 100.0f;

	// Up/Down arrow keys to control Z position
	if (IsKeyPressed(GLFW_KEY_UP))
		cameraPosition.z -= cameraSpeed * deltaTime / 100.0f;
	if (IsKeyPressed(GLFW_KEY_DOWN))
		cameraPosition.z += cameraSpeed * deltaTime / 100.0f;
	cameraPosition.z = min(MAX_CAMERA_Z_POSITION, max(MIN_CAMERA_Z_POSITION, cameraPosition.z));

	// Left and Right arrow keys to control camera's Y rotation
	float cameraRotationY = camera->GetRotationY();
	if (IsKeyPressed(GLFW_KEY_LEFT))
		cameraRotationY += cameraRotationSpeed * deltaTime / 100.0f;
	if (IsKeyPressed(GLFW_KEY_RIGHT))
		cameraRotationY -= cameraRotationSpeed * deltaTime / 100.0f;

	camera->SetPosition(cameraPosition);
	camera->SetRotationY(cameraRotationY);
	camera->SetRotationZ(cameraRotationZ);

	// Demo stuff for 3D perspective
	// Press O or P to toggle between Orthographic and Perspective projection
	if (IsKeyPressed(GLFW_KEY_O))
		camera->SetOrthographic(1000.0f, -4000.0f, 4000.0f);
	if (IsKeyPressed(GLFW_KEY_P))
		camera->SetPerspective(glm::radians(80.0f), 0.01f, GAME_CAMERA_PERSPECTIVE_FAR_BOUND);

	if (IsKeyPressed(GLFW_KEY_SPACE))
		point_camera_at_current_player();
}

bool WorldSystem::is_game_scene() {
	return ActiveScene == GameScene;
}

bool WorldSystem::is_menu_scene() {
	return ActiveScene == MenuScene;
}

bool WorldSystem::is_help_scene() {
	return ActiveScene == HelpScene;
}

bool WorldSystem::is_moving(ECS_ENTT::Entity entity) {
	vec2 velocity = entity.GetComponent<Motion>().velocity;
	return abs(velocity.x) > END_TURN_VELOCITY_X || abs(velocity.y) > END_TURN_VELOCITY_Y;
}

void WorldSystem::spawn_players() {
	// Assumes that each level has exactly 1 start tile
	auto id_tile = GameScene->m_Registry.view<StartTile>().back();
	auto e_tile = ECS_ENTT::Entity(id_tile, WorldSystem::GameScene);
	auto m_tile = e_tile.GetComponent<Motion>();

	// Create the bros at the start tile
	for (int i = 0; i < num_players; i++)
	{
		// Render players left to right, with first player in front
		float offset = SPRITE_SCALE / num_players;
		float x = m_tile.position.x + offset * i;
		float y = m_tile.position.y + offset;
		float z = 4.f * (i + 1); // Z-fighting
		const auto& create_slingBro = slingBroFunctions.at(i);
		auto player = (*create_slingBro)(glm::vec3(x, y, z), WorldSystem::GameScene);

		// Set the turn order of the player
		auto& turn = player.GetComponent<Turn>();
		turn.order = i;
	}
}

void WorldSystem::point_camera_at_current_player() {
	// Point camera at the bro only if the active scene is the GameScene
	if (!is_game_scene())
		return;

	auto slingbro = get_current_player();
	ActiveScene->PointCamera(slingbro.GetComponent<Motion>().position);
}

ECS_ENTT::Entity WorldSystem::get_current_player() {
	// Game scene must have the expected number of players
	assert(GameScene->m_Registry.view<SlingBro>().size() >= num_players);

	// Find the player with the current player index
	auto player_idx = GameScene->GetPlayer();
	auto slingbros_view = GameScene->m_Registry.view<SlingBro>();
	for (auto slingbro_id : slingbros_view)
	{
		auto player = ECS_ENTT::Entity(slingbro_id, WorldSystem::GameScene);
		if (player.GetComponent<Turn>().order == player_idx)
		{
			return player;
		}
	}

	// Fallback to assume view order is same as initial scene
	// render order, which is not true so I hope we don't get here
	return ECS_ENTT::Entity(slingbros_view[player_idx], WorldSystem::GameScene);
}

bool WorldSystem::should_end_turn(float elapsed_ms) {
	// Get current player turn
	auto current_player = get_current_player();
	auto& turn = current_player.GetComponent<Turn>();

	// Don't end the player's turn if they have yet to sling
	if (!turn.slung)
	{
		return false;
	}

	// Check if velocity within threshold, i.e. close to halt
	if (is_moving(current_player))
	{
		// Reset countdown
		turn.countdown = END_TURN_COUNTDOWN;
		return false;
	}

	// Progress turn countdown
	turn.countdown -= elapsed_ms;
	return turn.countdown <= 0.f;
}

void WorldSystem::save_turn_point_information(int* arr) {
	for (auto entityId : ActiveScene->m_Registry.view<Turn>()) {
		auto turn = ActiveScene->m_Registry.get<Turn>(entityId);
		arr[turn.order] = turn.points;
	}
}

void WorldSystem::load_turn_point_information(int turnPoints[]) {
	for (auto entityId : ActiveScene->m_Registry.view<Turn>()) {
		auto& turn = ActiveScene->m_Registry.get<Turn>(entityId);
		turn.points = turnPoints[turn.order];
	}
}

void WorldSystem::reward_current_player() {
	auto& currentPlayerTurn = get_current_player().GetComponent<Turn>();
	currentPlayerTurn.points += LEVEL_COMPLETION_REWARD;
}

unsigned int WorldSystem::set_next_player() {
	// Reset current player turn
	auto current_player = get_current_player();
	auto& turn = current_player.GetComponent<Turn>();
	turn.countdown = END_TURN_COUNTDOWN;
	turn.slung = false;

	// Increment player index to next player
	auto next_player_idx = (ActiveScene->GetPlayer() + 1) % num_players;
	WorldSystem::is_ai_turn = next_player_idx == 0;
	ActiveScene->SetPlayer(next_player_idx);
	
//	if (WorldSystem::is_ai_turn)
//	{
//		Mix_PlayChannel(-1, short_monster_sound, 0);
//	}

	// Pan camera to next player
	point_camera_at_current_player();
	return next_player_idx;
}

void WorldSystem::load_level(const std::string& level_file_path, bool spawn_players)
{
	// Check if level exists
	if (!file_exists(level_file_path))
	{
		printf("NO LEVEL NAMED '%s' FOUND! HAVE YOU CREATED THE LEVEL AND COPIED IT INTO THE levels/ DIRECTORY HUH????\n", level_file_path.c_str());
		assert(false); // todo(atsang): gracefully handle
	}

	// Load the level
	printf("Loading level from '%s'\n", level_file_path.c_str());
	Camera* oldCamera = new Camera(*WorldSystem::GameScene->GetCamera());
	delete(WorldSystem::GameScene);
	WorldSystem::GameScene = LevelManager::load_level(level_file_path, oldCamera);
	WorldSystem::ActiveScene = WorldSystem::GameScene;

	background_music = Mix_LoadMUS(audio_path(WorldSystem::GameScene->m_BackgroundMusicFileName).c_str());
	Mix_PlayMusic(background_music, -1);

	// Spawn the players
	if (spawn_players)
	{
		WorldSystem::spawn_players();
	}

	// Pan camera to next player
	point_camera_at_current_player();
}

void WorldSystem::reload_level()
{
	// User has saved level progress, so load the saved level file
	const std::string saved_file_path = saved_path(yaml_file(SAVE_FILE_NAME));
	if (file_exists(saved_file_path))
	{
		printf("Found saved data at '%s'\n", saved_file_path.c_str());
		load_level(saved_file_path, false);
		return;
	}

	// Otherwise, reload the current level
	load_level(levels_path(yaml_file(levels[get_level_number()])), true);
}

bool WorldSystem::file_exists(const std::string& file_path)
{
	// Try to open the file
	std::ifstream fin(file_path);

	// To exist or not to exist
	return !!fin;
}

glm::vec2 WorldSystem::getDispVecFromSource(glm::vec2 mouse_pos, glm::vec3 source_pos)
{
	Camera* activeCamera = WorldSystem::ActiveScene->GetCamera();
	glm::mat4 viewMatrix = activeCamera->GetViewMatrix();
	glm::mat4 projMatrix = activeCamera->GetProjectionMatrix();
	glm::mat4 textModelMatrix = glm::translate(glm::mat4(1), source_pos);
	glm::vec4 textPosClipSpace = projMatrix * viewMatrix * textModelMatrix * glm::vec4(1.0f);
	glm::vec2 textNDCS = glm::vec2(textPosClipSpace.x, textPosClipSpace.y) / textPosClipSpace.w;
	glm::vec2 sourcePosScreenSpace = glm::vec2(((textNDCS.x + 1.0f) / 2.0f) * WINDOW_SIZE_IN_PX.x, WINDOW_SIZE_IN_PX.y - ((textNDCS.y + 1.0f) / 2.0f) * WINDOW_SIZE_IN_PX.y);
	glm::vec2 dispVecFromSource = mouse_pos - sourcePosScreenSpace;
	return dispVecFromSource;
}

unsigned int WorldSystem::get_level_number()
{
	for (unsigned int i = 0; i < levels.size(); i++) {
		if (levels[i] == ActiveScene->m_Name) {
			return i;
		}
	}
	return 0;
}
