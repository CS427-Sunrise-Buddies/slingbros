#define _USE_MATH_DEFINES
#include <cmath>

// Header
#include "world.hpp"
#include "debug.hpp"
#include "render_components.hpp"
#include "animation.hpp" 
#include "loader/level_manager.hpp"

#include <glm/ext/matrix_transform.hpp>

// stlib
#include <string.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <entities/windy_grass.hpp>
#include <entities/grassy_tile.hpp>
#include <entities/lava_tile.hpp>
#include <entities/screen.hpp>
#include <entities/projectedPath.hpp>
#include <entities/sand_tile.hpp>
#include <entities/glass_tile.hpp>

// Entities
#include "entities/powerup.hpp"
#include "entities/speed_powerup.hpp"
#include "entities/slingbro.hpp"
#include "entities/projectile.hpp"
#include "entities/helge_projectile.hpp"
#include "entities/button.hpp"
#include "entities/goal_tile.hpp"
#include "entities/snail_enemy.hpp"
#include "entities/start_tile.hpp"
#include "entities/size_up_powerup.hpp"
#include "entities/size_down_powerup.hpp"
#include "entities/mass_up_powerup.hpp"
#include "entities/coin_powerup.hpp"
#include "entities/dialogue_box.hpp"
#include "entities/spike_hazard.hpp"
#include "entities/hazard_tile_spike.hpp"
#include "entities/bird_enemy.hpp"
#include "entities/basic_enemy.hpp"
#include "entities/blueb_enemy.hpp"
#include "menu_system.hpp"

// Game configuration
ECS_ENTT::Scene* WorldSystem::ActiveScene = nullptr;
std::stack<ECS_ENTT::Scene*> WorldSystem::PrevScenes;

// For this barebones template, this is just the main scene (Scenes are essentially just entity containers)
ECS_ENTT::Scene* WorldSystem::GameScene = nullptr;
ECS_ENTT::Scene* WorldSystem::MenuScene = nullptr;
ECS_ENTT::Scene* WorldSystem::HelpScene = nullptr;
ECS_ENTT::Scene* WorldSystem::FinaleScene = nullptr;

bool WorldSystem::is_ai_turn = true;

bool isLoadNextLevel = false;
bool isLevelRestart = false;

typedef ECS_ENTT::Entity (*fn)(vec3, ECS_ENTT::Scene*);
const std::vector<fn> slingBroFunctions = { SlingBro::createOrangeSlingBro, SlingBro::createPinkSlingBro };

// Create the slingBro world
// Note, this has a lot of OpenGL specific things, could be moved to the renderer; but it also defines the callbacks to the mouse and keyboard. That is why it is called here.
WorldSystem::WorldSystem(ivec2 window_size_px)
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

	WorldSystem::FinaleScene = new ECS_ENTT::Scene("Congratulations!", {10, 10});
	// Initialize finale camera properties
	Camera* finaleCamera = WorldSystem::FinaleScene->GetCamera();
	finaleCamera->SetViewportSize(WINDOW_SIZE_IN_PX.x, WINDOW_SIZE_IN_PX.y);
	finaleCamera->SetPosition(MENU_CAMERA_POSITION);
	finaleCamera->SetPerspective(glm::radians(60.0f), 0.01f, MENU_CAMERA_PERSPECTIVE_FAR_BOUND);
	finaleCamera->SetFixed(true);

	// Initialize Game on the Menu scene
	WorldSystem::ActiveScene = MenuScene;
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (win_music != nullptr)
		Mix_FreeMusic(win_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);
	if (yeehaw_sound != nullptr)
		Mix_FreeChunk(yeehaw_sound);
	if (ugh_sound != nullptr)
		Mix_FreeChunk(ugh_sound);
	if (poppin_click_sound != nullptr)
		Mix_FreeChunk(poppin_click_sound);
	if (disabled_click_sound != nullptr)
		Mix_FreeChunk(disabled_click_sound);
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
	if (sand_skidding_sound != nullptr)
		Mix_FreeChunk(sand_skidding_sound);
	if (tapping_glass_sound != nullptr)
		Mix_FreeChunk(tapping_glass_sound);
	if (snow_steppin_sound != nullptr)
		Mix_FreeChunk(snow_steppin_sound);
	Mix_CloseAudio();

	// Free memory of all of the game's scenes
	delete (GameScene);
	delete (HelpScene);
	delete (MenuScene);
	delete (FinaleScene);

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

	background_music = Mix_LoadMUS(audio_path("bgm_relax.wav").c_str());
	win_music = Mix_LoadMUS(audio_path("game_win.wav").c_str());
	yeehaw_sound = Mix_LoadWAV(audio_path("yeehaw.wav").c_str());
	ugh_sound = Mix_LoadWAV(audio_path("ugh.wav").c_str());
	poppin_click_sound = Mix_LoadWAV(audio_path("poppin_click.wav").c_str());
	disabled_click_sound = Mix_LoadWAV(audio_path("disabled_click.wav").c_str());
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
	sand_skidding_sound = Mix_LoadWAV(audio_path("skidding_sand.wav").c_str());
	tapping_glass_sound = Mix_LoadWAV(audio_path("glass_tap.wav").c_str());
	snow_steppin_sound = Mix_LoadWAV(audio_path("snow_steppin.wav").c_str());
	Mix_VolumeChunk(sand_skidding_sound, 60);
	Mix_VolumeChunk(tapping_glass_sound, 30);
	Mix_VolumeChunk(snow_steppin_sound, 30);
}


ECS_ENTT::Scene* WorldSystem::MenuInit()
{
	MenuSystem::init_start_menu(WorldSystem::MenuScene);
	return WorldSystem::MenuScene;
}

ECS_ENTT::Scene* WorldSystem::HelpInit()
{
	MenuSystem::init_help_menu(WorldSystem::HelpScene);
	return WorldSystem::HelpScene;
}

ECS_ENTT::Scene* WorldSystem::FinaleInit()
{
	MenuSystem::init_finale_menu(WorldSystem::FinaleScene);
	return WorldSystem::FinaleScene;
}

// Update our game world
ECS_ENTT::Scene* WorldSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << ActiveScene->m_Name;
	if (is_game_scene())
	{
		runWeatherCallbacks();
		if (is_ai_turn) {
			title_ss << " | Turn: Enemy";
		} else {
			auto turn = get_current_player().GetComponent<Turn>();
			auto countdown = ceil(turn.countdown / 100.f) / 10.f;
			title_ss << " | Turn: Player " << ActiveScene->GetPlayer() << " (end turn in " << countdown << "s)" << " | Points: " << turn.points;
		}
	}
	glfwSetWindowTitle(window, title_ss.str().c_str());

	for (auto entityID : GameScene->m_Registry.view<PlayerProfile>())
	{
		Text& text = GameScene->m_Registry.get<Text>(entityID);
		auto profile = GameScene->m_Registry.get<PlayerProfile>(entityID);
		int points = 0;
		text.colour = { 0.f, 0.f, 0.f };

		switch (profile.broType)
		{
			case BroType::ORANGE:
			{
				auto orange = GameScene->m_Registry.view<OrangeBro>()[0];
				points = GameScene->m_Registry.get<Turn>(orange).points;
				if (!is_ai_turn && orange == get_current_player())
				{
					text.colour = { 0.87f, 0.48f, 0.17f };
				}
				break;
			}
			case BroType::PINK:
			{
				auto pink = GameScene->m_Registry.view<PinkBro>()[0];
				points = GameScene->m_Registry.get<Turn>(pink).points;
				if (!is_ai_turn && pink == get_current_player())
				{
					text.colour = { 1.f, 0.51f, 0.77f };
				}
			}
		}
		text.content = profile.playerName + ": " + std::to_string(points);
	}

	if (!ActiveScene->GetDialogueBoxNames().empty() && !ActiveScene->is_in_dialogue) {
		ActiveScene->is_in_dialogue = true;
		handleDialogue();
	}

	if (is_game_scene())
	{
		// End player turn
		if (should_end_turn(elapsed_ms))
		{
			current_player_effects_tick();
			Mix_PlayChannel(-1, squeak_sound, 0);

			set_next_player();
		}
		else
		{
			update_projected_path(elapsed_ms);
		}

		// Point camera at the moving bro
		if (is_moving(get_current_player()))
		{
			point_camera_at_current_player();
		}
	}

	// TODO Removing out of screen entities (for the appropriate entities like projectiles, for example)

	// Tick all deformation component timers
	auto deformationView = ActiveScene->m_Registry.view<Deformation>();
	for (auto entityID : deformationView) {
		ECS_ENTT::Entity deformedEntity = ECS_ENTT::Entity(entityID, WorldSystem::ActiveScene);
		Deformation& deformation = deformedEntity.GetComponent<Deformation>();
		deformation.timeRemaining -= elapsed_ms;
		if (deformation.timeRemaining <= 0)
			deformedEntity.RemoveComponent<Deformation>();
	}

	// Tick all projectile component timers
	auto projectileView = ActiveScene->m_Registry.view<Projectile>();
	for (auto entityID : projectileView) {
		ECS_ENTT::Entity projectileEntity = ECS_ENTT::Entity(entityID, WorldSystem::ActiveScene);
		Projectile& projectile = projectileEntity.GetComponent<Projectile>();
		projectile.timeRemaining -= elapsed_ms;
		if (projectile.timeRemaining <= 0)
			ActiveScene->m_Registry.destroy(entityID);
	}

	// Tick all Helge projectile component timers
	auto helgeProjectileView = ActiveScene->m_Registry.view<HelgeProjectile>();
	for (auto entityID : helgeProjectileView) {
		ECS_ENTT::Entity helgeProjectileEntity = ECS_ENTT::Entity(entityID, WorldSystem::ActiveScene);
		HelgeProjectile& helgeProjectile = helgeProjectileEntity.GetComponent<HelgeProjectile>();
		helgeProjectile.timeRemaining -= elapsed_ms;
		if (helgeProjectile.timeRemaining <= 0)
			ActiveScene->m_Registry.destroy(entityID);
	}

	return ActiveScene;
}

void WorldSystem::update_projected_path(float elapsed_ms) {
	auto slingBro = get_current_player();
	auto& turn = slingBro.GetComponent<Turn>();
	if (slingBro.GetComponent<SlingMotion>().isClicked)
	{
		float minOpacity = 0.4;
		for (auto& entity : ActiveScene->m_Registry.view<ProjectedPath>())
		{
			auto star = ECS_ENTT::Entity(entity, GameScene);
			auto& shader = star.GetComponent<ShadedMeshRef>();
			float opacityShrinkScale = turn.path_fade_ms / PROJECTED_PATH_FADE_COUNTDOWN;

			float scale = star.GetComponent<ProjectedPath>().scale;

			shader.reference_to_cache->texture.color.w = opacityShrinkScale + minOpacity;
			opacityShrinkScale = max(opacityShrinkScale, minOpacity);
			star.GetComponent<Motion>().scale = { opacityShrinkScale * scale,
												  opacityShrinkScale * scale,
												  0 };
		}

		// update countdown and path for projected path
		if (turn.path_fade_ms < 0.f)
		{
			turn.path_fade_ms = PROJECTED_PATH_FADE_COUNTDOWN;
		}
		else
		{
			turn.path_fade_ms -= elapsed_ms;
		}
	}
}

void WorldSystem::handleDialogue() {
	get_current_player().GetComponent<SlingMotion>().canClick = false;
	ActiveScene->GetCamera()->SetFixed(true);
	
	// Remove previous box
	RemoveAllEntitiesWithComponent<DialogueBox>();

	std::queue<std::string> dialogue_boxes = ActiveScene->GetDialogueBoxNames();
	if (!dialogue_boxes.empty() && ActiveScene->is_in_dialogue) {
		// Create new box
		std::string current_box = dialogue_boxes.front();
		ActiveScene->PopDialogueBoxNames();
		DialogueBox::createDialogueBox(current_box, ActiveScene);
	} else {
		ActiveScene->is_in_dialogue = false;
		RemoveAllEntitiesWithComponent<DialogueBox>();

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

	if (current_player.HasComponent<MassChanged>()) {
		auto& massChangedComponent = current_player.GetComponent<MassChanged>();

		if (massChangedComponent.turnsRemaining <= 0) {
			current_player.RemoveComponent<MassChanged>();
			auto& massComponent = current_player.GetComponent<Mass>();
			massComponent.value = 1.0f;
		}
		else {
			massChangedComponent.turnsRemaining -= 1;
		}
	}
}

void WorldSystem::remove_all_entities() {
	GameScene->m_Registry.each([](const auto entityID, auto &&...) {
		GameScene->m_Registry.destroy(entityID);
	});
	ParticleSystem::GetInstance()->clearBeeSwarms();
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

bool WorldSystem::getIsLevelRestart() {
	return isLevelRestart;
}

void WorldSystem::setIsLevelRestart(bool b) {
	isLevelRestart = b;
}

void WorldSystem::load_next_level()
{
	unsigned int levelNum = get_level_number() + 1;
	if (levelNum >= levels.size()) {
		printf("You won!\n");

		// Clear history stack
		while (!PrevScenes.empty())
		{
			PrevScenes.pop();
		}

		// Switch to Finale scene
		PrevScenes.push(ActiveScene);
		ActiveScene = FinaleScene;

		// Play some cool music
		Mix_PlayMusic(win_music, 0);
		return;
	}

	std::cout << "loading next level\n";

	std::vector<int> turnPoints(WorldSystem::GameScene->GetNumPlayers(), 0);
	reward_current_player();
	save_turn_point_information(&turnPoints);

	// The first player to go on the next level
	set_next_player();

	// Remove all old entities in the scene
	remove_all_entities();

	// Load the next level
	const std::string level_file_path = levels_path(yaml_file(levels[levelNum]));
	load_level(level_file_path, GameScene->GetNumPlayers());

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

	// Example usage of create_level as a dev tool
	// 1. Clone data/create/tutorial.yaml to make a new map
	// 2. Call create_level on the new file to generate the level yaml file
	// 3. Run the game
	// 4. Copy over the generated level yaml file from the build directory to the repo
	// 5. Add the file name (minus the .yaml extension) to data/config/config.yaml in whatever order
	// LevelManager::create_level("YourNewLevel.yaml")
	// Uncomment below to regenerate all levels
	// for (const auto& level_name : LevelManager::get_levels(NUM_PLAYERS_1)) LevelManager::create_level(yaml_file(level_name));
	// for (const auto& level_name : LevelManager::get_levels(NUM_PLAYERS_2)) LevelManager::create_level(yaml_file(level_name));

	// Reload the current level
	if (is_game_scene())
	{
		WorldSystem::load_level(levels_path(yaml_file(levels[get_level_number()])), GameScene->GetNumPlayers());
	}
}

void WorldSystem::powerup_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, ECS_ENTT::Scene* gameScene) {
	Mix_PlayChannel(-1, power_up_sound, 0);

	if (entity_j.HasComponent<SpeedPowerUp>()) {
		SpeedPowerUp& speed_power_up = entity_j.GetComponent<SpeedPowerUp>();
		speed_power_up.applyPowerUp(entity_i);
	} else if (entity_j.HasComponent<SizeUpPowerUp>()) {
		SizeUpPowerUp& size_up_power_up = entity_j.GetComponent<SizeUpPowerUp>();
		size_up_power_up.applyPowerUp(entity_i);
	} else if (entity_j.HasComponent<SizeDownPowerUp>()) {
		SizeDownPowerUp& size_down_power_up = entity_j.GetComponent<SizeDownPowerUp>();
		size_down_power_up.applyPowerUp(entity_i);
	} else if (entity_j.HasComponent<CoinPowerUp>()) {
		CoinPowerUp& coin_power_up = entity_j.GetComponent<CoinPowerUp>();
		coin_power_up.applyPowerUp(entity_i);
	} else if (entity_j.HasComponent<MassUpPowerUp>()) {
		MassUpPowerUp& mass_up_power_up = entity_j.GetComponent<MassUpPowerUp>();
		mass_up_power_up.applyPowerUp(entity_i);
	}

	gameScene->m_Registry.destroy(entity_j);
}

void WorldSystem::collidable_enemy_collision_listener(ECS_ENTT::Entity slingBroEntity, ECS_ENTT::Entity enemyEntity)
{
	Motion& slingBroMotion = slingBroEntity.GetComponent<Motion>();
	Motion& enemyMotion = enemyEntity.GetComponent<Motion>();

	// Don't count collisions when it is the enemy's turn
	if (is_ai_turn)
		return;

	auto& collidingBrosTurn = slingBroEntity.GetComponent<Turn>();
	collidingBrosTurn.subPoints(POINTS_LOST_UPON_ENEMY_COLLISION);

	// Knock the player away from the enemy and emit a "blood spray" particle effect 
	glm::vec2 dispVecNorm = glm::vec2(glm::normalize(enemyMotion.position - slingBroMotion.position));
	slingBroMotion.position -= 5.0f * glm::vec3(dispVecNorm.x, dispVecNorm.y, 0.0f);
	auto& broVelocity = slingBroMotion.velocity;
	float broSpeed = glm::length(broVelocity);
	broVelocity = broSpeed * -glm::vec3(dispVecNorm.x, dispVecNorm.y, 0.0f);
	broVelocity.x += (0.5f - Random::Float()) * MAX_VELOCITY / 2.0f;
	if (broSpeed > MAX_VELOCITY)
		broVelocity = (broVelocity / broSpeed) * MAX_VELOCITY;

	// Check orientation of character from the enemy and set knockback direction, particle velocity, & deformations correctly
	float angle = atan(dispVecNorm.y, dispVecNorm.x); // angle in radians from spike tile to slingbro
	glm::vec3 particleVelocity = glm::vec3(1.0f);
	float particleSpeed = 0.1f;
	if (angle > -PI / 4 && angle <= PI / 4) // bro to the right 
		particleVelocity = glm::vec3(-particleSpeed, 0.0f, 0.0f);
	else if (angle > PI / 4 && angle <= 3 * PI / 4)	// bro below 
		particleVelocity = glm::vec3(0.0f, -particleSpeed, 0.0f);
	else if (angle > 3 * PI / 4 && angle <= 5 * PI / 4)	// bro to the left 
		particleVelocity = glm::vec3(particleSpeed, 0.0f, 0.0f);
	else // bro above 
		particleVelocity = glm::vec3(0.0f, particleSpeed, 0.0f);

	// Deform the bro
	if (!slingBroEntity.HasComponent<Deformation>())
		slingBroEntity.AddComponent<Deformation>(0.8f, 1.2f, angle, 100.0f);
	Mix_PlayChannel(-1, ugh_sound, 0);

	// Emit blood spray particles
	ParticleSystem* particleSystem = ParticleSystem::GetInstance();
	ParticleProperties particle;
	glm::vec2 particleOffset = glm::vec2(dispVecNorm.x * (slingBroMotion.scale.x / 1.4f), dispVecNorm.y * (slingBroMotion.scale.y / 1.4f));
	particle.position = slingBroMotion.position; +glm::vec3(particleOffset.x, particleOffset.y, 0.0f);
	particle.velocity = broVelocity / (100.0f * broSpeed);
	particle.velocityVariation = glm::vec3(0.25f, 0.1f, 0.2f);
	particle.colourBegin = glm::vec4(0.9f, 0.1f, 0.1f, 1.0f);
	particle.colourEnd = glm::vec4(0.5f, 0.0f, 0.0f, 0.0f);
	particle.sizeBegin = 8.0f;
	particle.sizeEnd = 0.1f;
	particle.sizeVariation = 2.0f;
	particle.lifeTimeMs = 2000.0f;
	for (int i = 0; i < 100; i++)
		particleSystem->Emit(particle);
}

void WorldSystem::ground_spike_collision_listener(ECS_ENTT::Entity slingBroEntity, ECS_ENTT::Entity groundSpikeEntity)
{
	Motion& slingBroMotion = slingBroEntity.GetComponent<Motion>();
	Motion& groundSpikeMotion = groundSpikeEntity.GetComponent<Motion>();

	auto& collidingBrosTurn = slingBroEntity.GetComponent<Turn>();
	collidingBrosTurn.subPoints(POINTS_LOST_UPON_SPIKE_COLLISION);

	// Knock the player away from the spike and emit a "blood spray" particle effect 
	glm::vec2 dispVecNorm = glm::vec2(glm::normalize(groundSpikeMotion.position - slingBroMotion.position));
	slingBroMotion.position -= 1.0f * glm::vec3(dispVecNorm.x, dispVecNorm.y, 0.0f);
	auto& broVelocity = slingBroMotion.velocity;
	float broSpeed = glm::length(broVelocity);
	if (broVelocity.y > 0.0f)
		broVelocity.y *= -1.0f - (Random::Float() / 2.0f);
	broVelocity.x *= -1.0f + ((0.5f - Random::Float()) / 2.0f);
	if (abs(broVelocity.x) < 200.0f)
		broVelocity.x += (0.5f - Random::Float()) * 1.5f * glm::length(broVelocity);
	if (broSpeed > MAX_VELOCITY)
		broVelocity = (broVelocity / broSpeed) * MAX_VELOCITY;

	// Deform the character
	float angle = atan(dispVecNorm.y, dispVecNorm.x); // angle in radians from ground spike to slingbro
	if (!slingBroEntity.HasComponent<Deformation>())
		slingBroEntity.AddComponent<Deformation>(0.8f, 1.2f, angle, 100.0f);
	Mix_PlayChannel(-1, ugh_sound, 0);

	// Emit blood spray particles
	ParticleSystem* particleSystem = ParticleSystem::GetInstance();
	ParticleProperties particle;
	glm::vec2 particleOffset = glm::vec2(dispVecNorm.x * (slingBroMotion.scale.x / 1.4f), dispVecNorm.y * (slingBroMotion.scale.y / 1.4f));
	particle.position = slingBroMotion.position + glm::vec3(particleOffset.x, particleOffset.y, 0.0f);
	particle.velocity = glm::vec3(0.0f, -0.05f, 0.0f);
	particle.velocityVariation = glm::vec3(0.2f, 0.125f, 0.2f) / 2.0f;
	particle.colourBegin = glm::vec4(0.9f, 0.1f, 0.1f, 1.0f);
	particle.colourEnd = glm::vec4(0.5f, 0.0f, 0.0f, 0.0f);
	particle.sizeBegin = 8.0f;
	particle.sizeEnd = 4.0f;
	particle.sizeVariation = 2.0f;
	particle.lifeTimeMs = 4000.0f;
	for (int i = 0; i < 20; i++)
		particleSystem->Emit(particle);
	particle.velocity = broVelocity / (10.0f * broSpeed);
	particle.velocityVariation = glm::vec3(0.25f, 0.1f, 0.2f);
	for (int i = 0; i < 80; i++)
		particleSystem->Emit(particle);
}

void WorldSystem::tile_spike_collision_listener(ECS_ENTT::Entity slingBroEntity, ECS_ENTT::Entity tileSpikeEntity)
{
	Motion& slingBroMotion = slingBroEntity.GetComponent<Motion>();
	Motion& tileSpikeMotion = tileSpikeEntity.GetComponent<Motion>();

	auto& collidingBrosTurn = slingBroEntity.GetComponent<Turn>();
	collidingBrosTurn.subPoints(POINTS_LOST_UPON_SPIKE_COLLISION);

	// Knock the player away from the spike tile and emit a "blood spray" particle effect 
	glm::vec2 dispVecNorm = glm::vec2(glm::normalize(tileSpikeMotion.position - slingBroMotion.position));
	slingBroMotion.position += 2.0f * glm::vec3(dispVecNorm.x, dispVecNorm.y, 0.0f);
	auto& broVelocity = slingBroMotion.velocity;
	float broSpeed = glm::length(broVelocity);
	broVelocity = broSpeed * -glm::vec3(dispVecNorm.x, dispVecNorm.y, 0.0f);
	broVelocity.x += (0.5f - Random::Float()) * MAX_VELOCITY / 2.0f;
	if (broSpeed > MAX_VELOCITY)
		broVelocity = (broVelocity / broSpeed) * MAX_VELOCITY;

	// Check orientation of character from the spike tile and set knockback direction, particle velocity correctly
	float angle = atan(dispVecNorm.y, dispVecNorm.x); // angle in radians from spike tile to slingbro
	glm::vec3 particleVelocity = glm::vec3(1.0f);
	float particleSpeed = 0.1f;
	if (angle > -PI / 4 && angle <= PI / 4) // bro to the right of spike tile
		particleVelocity = glm::vec3(-particleSpeed, 0.0f, 0.0f);
	else if (angle > PI / 4 && angle <= 3 * PI / 4)	// bro below spike tile
		particleVelocity = glm::vec3(0.0f, -particleSpeed, 0.0f);
	else if (angle > 3 * PI / 4 && angle <= 5 * PI / 4)	// bro to the left of spike tile
		particleVelocity = glm::vec3(particleSpeed, 0.0f, 0.0f);
	else // bro above spike tile
		particleVelocity = glm::vec3(0.0f, particleSpeed, 0.0f);

	// Deform the character
	if (!slingBroEntity.HasComponent<Deformation>())
		slingBroEntity.AddComponent<Deformation>(0.8f, 1.2f, angle, 100.0f);
	Mix_PlayChannel(-1, ugh_sound, 0);

	// Emit blood spray particles
	ParticleSystem* particleSystem = ParticleSystem::GetInstance();
	ParticleProperties particle;
	glm::vec2 particleOffset = glm::vec2(dispVecNorm.x * (slingBroMotion.scale.x / 1.4f), dispVecNorm.y * (slingBroMotion.scale.y / 1.4f));
	particle.position = slingBroMotion.position + glm::vec3(particleOffset.x, particleOffset.y, 0.0f);
	particle.velocity = particleVelocity;
	particle.velocityVariation = glm::vec3(0.2f, 0.125f, 0.2f) / 2.0f;
	particle.colourBegin = glm::vec4(0.9f, 0.1f, 0.1f, 1.0f);
	particle.colourEnd = glm::vec4(0.5f, 0.0f, 0.0f, 0.0f);
	particle.sizeBegin = 8.0f;
	particle.sizeEnd = 4.0f;
	particle.sizeVariation = 2.0f;
	particle.lifeTimeMs = 4000.0f;
	for (int i = 0; i < 20; i++)
		particleSystem->Emit(particle);
	particle.velocity = broVelocity / (10.0f * broSpeed);
	particle.velocityVariation = glm::vec3(0.25f, 0.1f, 0.2f);
	for (int i = 0; i < 80; i++)
		particleSystem->Emit(particle);
}

void WorldSystem::projectile_collision_listener(ECS_ENTT::Entity slingBroEntity, ECS_ENTT::Entity projectileEntity)
{
	Motion& slingBroMotion = slingBroEntity.GetComponent<Motion>();
	Motion& projectileMotion = projectileEntity.GetComponent<Motion>();

	auto& collidingBrosTurn = slingBroEntity.GetComponent<Turn>();
	collidingBrosTurn.subPoints(POINTS_LOST_BASIC_PROJECTILE);
	ActiveScene->m_Registry.destroy(projectileEntity);

	// Knock the player away from the projectile
	glm::vec2 dispVecNorm = glm::vec2(glm::normalize(projectileMotion.position - slingBroMotion.position));
	slingBroMotion.position -= 2.0f * glm::vec3(dispVecNorm.x, dispVecNorm.y, 0.0f);
	auto& broVelocity = slingBroMotion.velocity;
	auto& projectileVelocity = projectileMotion.velocity;
	float broSpeed = glm::length(broVelocity);
	float projectileSpeed = glm::length(projectileVelocity);
	broVelocity = (broSpeed + projectileSpeed) * -2.0f * glm::vec3(dispVecNorm.x, dispVecNorm.y, 0.0f);

	if (broSpeed > MAX_PROJECTILE_KNOCKBACK_VELOCITY)
		broVelocity = (broVelocity / broSpeed) * MAX_PROJECTILE_KNOCKBACK_VELOCITY;

	// Deform the character
	float angle = atan(dispVecNorm.y, dispVecNorm.x);
	if (!slingBroEntity.HasComponent<Deformation>())
		slingBroEntity.AddComponent<Deformation>(0.8f, 1.2f, angle, 100.0f);
	Mix_PlayChannel(-1, ugh_sound, 0);
}

void WorldSystem::helge_projectile_collision_listener(ECS_ENTT::Entity slingBroEntity, ECS_ENTT::Entity helgeProjectileEntity)
{
	Motion& slingBroMotion = slingBroEntity.GetComponent<Motion>();
	Motion& helgeProjectileMotion = helgeProjectileEntity.GetComponent<Motion>();

	auto& turn = slingBroEntity.GetComponent<Turn>();
	turn.subPoints(POINTS_LOST_HELGE_PROJECTILE);
	ActiveScene->m_Registry.destroy(helgeProjectileEntity);

	// Knock the player away from Helge's projectile 
	glm::vec2 dispVecNorm = glm::vec2(glm::normalize(helgeProjectileMotion.position - slingBroMotion.position));
	slingBroMotion.position -= 2.0f * glm::vec3(dispVecNorm.x, dispVecNorm.y, 0.0f);
	auto& broVelocity = slingBroMotion.velocity;
	auto& projectileVelocity = helgeProjectileMotion.velocity;
	float broSpeed = glm::length(broVelocity);
	float projectileSpeed = glm::length(projectileVelocity);
	broVelocity = (broSpeed + projectileSpeed) * -2.0f * glm::vec3(dispVecNorm.x, dispVecNorm.y, 0.0f);

	if (broSpeed > MAX_PROJECTILE_KNOCKBACK_VELOCITY)
		broVelocity = (broVelocity / broSpeed) * MAX_PROJECTILE_KNOCKBACK_VELOCITY;

	// Deform the character
	float angle = atan(dispVecNorm.y, dispVecNorm.x);
	if (!slingBroEntity.HasComponent<Deformation>())
		slingBroEntity.AddComponent<Deformation>(0.6f, 1.4f, angle, 100.0f);
	Mix_PlayChannel(-1, ugh_sound, 0);
}

// Collisions between wall and non-wall entities - callback function, listening to PhysicsSystem::Collisions
// example of observer pattern
void WorldSystem::collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall)
{
	if (!is_game_scene()) return;

	if (entity_i.HasComponent<SlingBro>() && DebugSystem::in_debug_mode) {
		DebugSystem::in_freeze_mode = true;
	}

	auto current_player = get_current_player();

	if (!hit_wall && entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<Projectile>()) {
		projectile_collision_listener(entity_i, entity_j);
	}
	else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<HelgeProjectile>()) {
		helge_projectile_collision_listener(entity_i, entity_j);
	}
	else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<HazardSpike>()) {
		ground_spike_collision_listener(entity_i, entity_j);
	}
	else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<HazardTileSpike>()) {
		tile_spike_collision_listener(entity_i, entity_j);
	}
	else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<CollidableEnemy>()) {
		collidable_enemy_collision_listener(entity_i, entity_j);
	}
	else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<SlingBro>()) {
		Mix_PlayChannel(-1, ugh_sound, 0);
	}
	else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<PowerUp>()) {
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
		setIsLevelRestart(true);
	}
	else if (is_moving(current_player) && entity_i == current_player)
	{
		// slingBro sound effects for tiles
		if (entity_i.HasComponent<SlingBro>())
		{
			if (abs(entity_i.GetComponent<Motion>().velocity.y) > AUDIO_TRIGGER_VELOCITY_Y) {
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
				else if (entity_j.HasComponent<SandTile>())
				{
					Mix_PlayChannel(-1, sand_skidding_sound, 0);
				}
				else if (entity_j.HasComponent<GlassTile>())
				{
					Mix_PlayChannel(-1, tapping_glass_sound, 0);
				}
				else if (entity_j.HasComponent<BouncyTile>())
				{
				Mix_PlayChannel(-1, snow_steppin_sound, 0);
				}
				else if (hit_wall)
				{
					Mix_PlayChannel(-1, short_thud_sound, 0);
				}
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
	}
	else if (is_help_scene())
	{
		// Toggle help menu
		if (action == GLFW_RELEASE && (key == GLFW_KEY_H || key == GLFW_KEY_ESCAPE))
		{
			ECS_ENTT::Scene* prev = PrevScenes.top();
			PrevScenes.pop();
			PrevScenes.push(ActiveScene);
			ActiveScene = prev;
		}
	}
	// A game scene is active
	else if (is_game_scene() && ActiveScene->is_in_dialogue)
	{
		if (action == GLFW_RELEASE) {
			// Restart current level
			if (key == GLFW_KEY_R)
			{
				restart();
			}

			// Restart from saved state
			if (key == GLFW_KEY_L)
			{
				load_saved_level();
			}

			// Bring up help scene
			else if (key == GLFW_KEY_H)
			{
				PrevScenes.push(ActiveScene);
				ActiveScene = HelpScene;
			}

			// Return to main menu
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

			// Hidden skip level key
			else if (key == GLFW_KEY_0)
			{
				Mix_PlayChannel(-1, transport_sound, 0);
				Mix_PlayChannel(-1, yeehaw_sound, 0);
				setIsLoadNextLevel(true);
				Camera* activeCamera = WorldSystem::GameScene->GetCamera();
				activeCamera->SetFixed(false);
			}

			// Progress dialogue
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

			// Restart from saved state
			if (key == GLFW_KEY_L)
			{
				load_saved_level();
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

			// Hidden skip level key
			else if (key == GLFW_KEY_0)
			{
				Mix_PlayChannel(-1, transport_sound, 0);
				Mix_PlayChannel(-1, yeehaw_sound, 0);
				setIsLoadNextLevel(true);
				Camera* activeCamera = WorldSystem::GameScene->GetCamera();
				activeCamera->SetFixed(false);
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
	if (is_menu_scene() || is_help_scene() || is_finale_scene())
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

	if (is_game_scene())
	{
		auto slingBro = get_current_player();

		if (slingBro.GetComponent<SlingMotion>().isClicked && !slingBro.GetComponent<Turn>().slung)
		{
			// Play sling back sfx
			Mix_PlayChannel(-1, balloon_tap_sound, 0);
			// refresh the projected path
			draw_projected_path(slingBro, mouse_pos);
		}

		// Rotate the bro
		if (!is_ai_turn && !ActiveScene->is_in_dialogue)
		{
			if (!slingBro.HasComponent<DeathTimer>())
			{
				auto& m_slingbro = slingBro.GetComponent<Motion>();
				glm::vec2 dispVecFromBro = getDispVecFromSource(glm::vec2(mouse_pos), glm::vec3(m_slingbro.position));
				float angle = atan2(dispVecFromBro.y, dispVecFromBro.x);
				// Offset angle by pi/2 to correct rotation angle
				m_slingbro.angle = angle + M_PI_2;
			}
		}
	}

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

			auto& turn = slingbro.GetComponent<Turn>();

			// Start sling
			if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			{
				// Keep track of the mouse position of the click
				clickPosition = mouse_pos;

				if (abs(dispVecFromBro.x) < broMotion.scale.x && abs(dispVecFromBro.y) < broMotion.scale.y && canClick && !turn.slung)
				{
					isBroClicked = !WorldSystem::is_ai_turn;
				}
			}

			// End sling
			else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && isBroClicked)
			{
				// Reset clicked
				isBroClicked = false;

				// Already had a turn
				if (turn.slung)
				{
					return;
				}

				// Don't count clicks with no drag
				if (length(mouse_pos - clickPosition) < MIN_DRAG_LENGTH)
				{
					return;
				}

				RemoveAllEntitiesWithComponent<ProjectedPath>();

				dragDir = -dispVecFromBro;
				dragMagnitude *= length(dragDir);
				dragDir.x *= dragMagnitude.x;
				dragDir.y *= dragMagnitude.y;
				broVelocity = vec3(dragDir, 0.0) / 1000.f;

				// setting max speed for the bro, maybe load the velocities when we have the level loader
				if (glm::length(broVelocity) > MAX_VELOCITY)
					broVelocity = glm::normalize(broVelocity) * MAX_VELOCITY;

				// Record that player has slung
				turn.slung = true;
			}
		}
	}

	if (is_menu_scene() || is_help_scene() || is_finale_scene())
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		{
			// Find the button in the scene that the cursor is hovered over
			for (auto entity : WorldSystem::ActiveScene->m_Registry.view<ClickableText>())
			{
				ClickableText& clickableText = ActiveScene->m_Registry.get<ClickableText>(entity);
				if (clickableText.isHoveredOver && !clickableText.isClicked)
				{
					// Set to clicked
					clickableText.isClicked = true;

					// Do the button action
					click_button(clickableText);

					// Click only one button at a time
					break;
				}
			}
		}
	}
}

void WorldSystem::click_button(ClickableText& button)
{
	// Reset clicked attribute
	button.isClicked = false;

	if (is_menu_scene())
	{
		// Start a new game from scratch and choose number of players
		if (button.functionName == BUTTON_NAME_NEW)
		{
			// Set to player selection menu
			MenuSystem::set_main_menu(WorldSystem::MenuScene, MainMenuType::PLAYER_SELECTION);
		}
		// Start a new 1-player game
		else if (button.functionName == BUTTON_NAME_1P)
		{
			remove_all_entities();
			// Unfreeze camera
			Camera* activeCamera = WorldSystem::GameScene->GetCamera();
			activeCamera->SetFixed(false);
			// Load the first level and spawn 1 player
			levels = LevelManager::get_levels(NUM_PLAYERS_1);
			GameScene->SetPlayer(0);
			load_level(levels_path(yaml_file(levels[0])), NUM_PLAYERS_1);
		}
		// Start a new 2-player game
		else if (button.functionName == BUTTON_NAME_2P)
		{
			remove_all_entities();
			// Unfreeze camera
			Camera* activeCamera = WorldSystem::GameScene->GetCamera();
			activeCamera->SetFixed(false);
			// Load the first level and spawn 2 players
			levels = LevelManager::get_levels(NUM_PLAYERS_2);
			load_level(levels_path(yaml_file(levels[0])), NUM_PLAYERS_2);
		}
		// Back to start menu
		else if (button.functionName == BUTTON_NAME_BACK)
		{
			// Set to start menu
			MenuSystem::set_main_menu(WorldSystem::MenuScene, MainMenuType::START);
		}
		// Continue your saved game
		else if (button.functionName == BUTTON_NAME_RESUME)
		{
			remove_all_entities();
			if (load_saved_level())
			{
				ActiveScene = GameScene;
				levels = LevelManager::get_levels(GameScene->GetNumPlayers());
				point_camera_at_current_player();
			}
		}
		// Ain't no saved game
		else if (button.functionName == BUTTON_NAME_RESUME_DISABLED)
		{
			// Play unable to click SFX
			Mix_PlayChannel(-1, disabled_click_sound, 0);
			return;
		}
		// Quit the game and close the window
		else if (button.functionName == BUTTON_NAME_QUIT)
		{
			printf("Exit game\n");
			exit(0);
		}
		// Open the help menu
		else if (button.functionName == BUTTON_NAME_HELP)
		{
			PrevScenes.push(ActiveScene);
			ActiveScene = HelpScene;
		}
	}

	else if (is_help_scene())
	{
		// Previous help page
		if (button.functionName == BUTTON_NAME_PREV)
		{
			MenuSystem::set_help_menu(HelpScene, PageDirection::PREV);
		}
		// Next help page
		else if (button.functionName == BUTTON_NAME_NEXT)
		{
			MenuSystem::set_help_menu(HelpScene, PageDirection::NEXT);
		}
	}

	else if (is_finale_scene())
	{
		// Back to start menu
		if (button.functionName == BUTTON_NAME_BACK)
		{
			// Set to start menu
			MenuSystem::set_main_menu(WorldSystem::MenuScene, MainMenuType::START);
			PrevScenes.push(ActiveScene);
			ActiveScene = MenuScene;
		}
		// Quit game
		else if (button.functionName == BUTTON_NAME_QUIT_SMALL)
		{
			printf("Exit game\n");
			exit(0);
		}
	}

	// Play click SFX
	Mix_PlayChannel(-1, poppin_click_sound, 0);
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

bool WorldSystem::is_finale_scene() {
	return ActiveScene == FinaleScene;
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

	// Amount of space between each player
	float offset = SPRITE_SCALE / 2.f;

	// First player should be on the rightmost, so the
	// leftmost player must be the next player index
	auto num_players = GameScene->GetNumPlayers();
	auto first_player = GameScene->GetPlayer();
	auto second_player = (first_player + 1) % num_players;

	// Create the bros at the start tile
	for (int i = 0; i < num_players; i++)
	{
		// Calculate index starting from leftmost player
		auto curr_player = (second_player + i) % num_players;

		// Render players left to right, back to front
		float x = m_tile.position.x + offset * i;
		float y = m_tile.position.y + offset;
		float z = 4.f * (i + 1); // Z-fighting
		const auto& create_slingBro = slingBroFunctions.at(curr_player);
		auto bro = (*create_slingBro)(glm::vec3(x, y, z), WorldSystem::GameScene);

		// Set the turn order of the player where
		// order goes from right to left
		auto& turn = bro.GetComponent<Turn>();
		turn.order = curr_player;
	}
}

void WorldSystem::point_camera_at_current_player() {
	auto slingbro = get_current_player();
	GameScene->PointCamera(slingbro.GetComponent<Motion>().position);
}

ECS_ENTT::Entity WorldSystem::get_current_player() {
	// Game scene must have the expected number of players
	auto slingbros_view = GameScene->m_Registry.view<SlingBro>();
	assert(slingbros_view.size() == GameScene->GetNumPlayers());

	// Find the player with the current player index
	auto player_idx = GameScene->GetPlayer();
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

	// End the turn if the player has lost too many points this turn
	if (turn.pointsLostThisTurn >= MAX_POINTS_LOST_PER_TURN)
		return true;

	// Don't end the player's turn if they have yet to sling
	if (!turn.slung)
		return false;
	

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

void WorldSystem::save_turn_point_information(std::vector<int>* arr) {
	for (auto entityId : ActiveScene->m_Registry.view<Turn>()) {
		auto turn = ActiveScene->m_Registry.get<Turn>(entityId);
		(*arr)[turn.order] = turn.points;
	}
}

void WorldSystem::load_turn_point_information(std::vector<int> turnPoints) {
	for (auto entityId : ActiveScene->m_Registry.view<Turn>()) {
		auto& turn = ActiveScene->m_Registry.get<Turn>(entityId);
		turn.points = turnPoints[turn.order];
	}
}

void WorldSystem::reward_current_player() {
	auto& currentPlayerTurn = get_current_player().GetComponent<Turn>();
	currentPlayerTurn.addPoints(POINTS_GAINED_ON_WIN);
}

void WorldSystem::decrement_points_bee_swarm() {
	auto activePlayerEntity = get_current_player();
	auto& currentPlayersTurn = activePlayerEntity.GetComponent<Turn>();
	currentPlayersTurn.subPoints(POINTS_LOST_PER_TURN_PER_BEE_SWARM * ParticleSystem::GetInstance()->NumBeesTargetingEntity(activePlayerEntity.GetEntityID()));
}

unsigned int WorldSystem::set_next_player() {
	// Reset current player turn
	auto current_player = get_current_player();
	auto& turn = current_player.GetComponent<Turn>();
	turn.countdown = END_TURN_COUNTDOWN;
	turn.slung = false;
	turn.path_fade_ms = PROJECTED_PATH_FADE_COUNTDOWN;

	// Increment player index to next player
	auto next_player_idx = (GameScene->GetPlayer() + 1) % GameScene->GetNumPlayers();
	WorldSystem::is_ai_turn = next_player_idx == 0 && !GameScene->m_Registry.view<AI>().empty();
	GameScene->SetPlayer(next_player_idx);

	// Get new player's turn and reset their pointsLostThisTurn to zero
	auto& newPlayersTurn = get_current_player().GetComponent<Turn>();
	newPlayersTurn.pointsLostThisTurn = 0;

	// Take away points for every bee swarm targeting the current player
	decrement_points_bee_swarm();
	
//	if (WorldSystem::is_ai_turn)
//	{
//		Mix_PlayChannel(-1, short_monster_sound, 0);
//	}

	// Pan camera to next player
	point_camera_at_current_player();
	return next_player_idx;
}

void WorldSystem::load_level(const std::string& level_file_path, size_t num_players_to_spawn)
{
	// Check if level exists
	assert(Util::file_exists(level_file_path) && "HAVE YOU CREATED THE LEVEL AND COPIED IT INTO THE levels/ DIRECTORY HUH????\n");

	// Grab old scene values and delete
	auto next_player_idx = GameScene->GetPlayer();
	Camera* oldCamera = new Camera(*WorldSystem::GameScene->GetCamera());
	delete(WorldSystem::GameScene);

	// Load the level
	printf("Loading level from '%s'\n", level_file_path.c_str());
	WorldSystem::GameScene = LevelManager::load_level(level_file_path, oldCamera);
	GameScene->SetPlayer(next_player_idx);

	// Spawn the players
	if (num_players_to_spawn > 0)
	{
		WorldSystem::GameScene->SetNumPlayer(num_players_to_spawn);
		WorldSystem::spawn_players();
	}


	for (auto entityID : GameScene->m_Registry.view<SlingBro>())
	{
		auto player = ECS_ENTT::Entity(entityID, GameScene);

		std::shared_ptr<TextFont> font = TextFont::load(RETRO_COMPUTER_TTF);

		// sorry for hardcoding T_T but there's only 2 of em
		if (player.HasComponent<OrangeBro>())
		{
			auto bro = SlingBro::createOrangeSlingBroProfile(glm::vec3(30, 20, 0), WorldSystem::GameScene);
			Text& playerTextComponent = bro.GetComponent<Text>();
			playerTextComponent.font = font;
			playerTextComponent.scale = TEXT_SCALE;
			playerTextComponent.position = vec2(TEXT_DISTANCE_X, TEXT_DISTANCE_Y);
		}
		else
		{
			auto bro = SlingBro::createPinkSlingBroProfile(glm::vec3(30, 20, 0), WorldSystem::GameScene);
			Text& playerTextComponent = bro.GetComponent<Text>();
			playerTextComponent.font = font;
			playerTextComponent.scale = TEXT_SCALE;
			playerTextComponent.position = vec2(TEXT_DISTANCE_X, TEXT_DISTANCE_Y * 2);
		}
	}


	// Pan camera to next player
	point_camera_at_current_player();

	// Play music
	background_music = Mix_LoadMUS(audio_path(WorldSystem::GameScene->m_BackgroundMusicFileName).c_str());
	Mix_PlayMusic(background_music, -1);

	// Switch to game scene
	WorldSystem::ActiveScene = WorldSystem::GameScene;

	// create title text
	std::shared_ptr<TextFont> font = TextFont::load(RETRO_COMPUTER_TTF);
	createText("texty", GameScene->m_Name, {30, 770}, font, 0.5f);
}

ECS_ENTT::Entity WorldSystem::createText(std::string name, std::string content, vec2 position, std::shared_ptr<TextFont> font, float scale)
{
	ECS_ENTT::Entity texty = GameScene->CreateEntity(name);
	Text& textComponent = texty.AddComponent<Text>();
	textComponent.font = font;
	textComponent.scale = scale;
	textComponent.position = position;
	textComponent.content = content;
	texty.AddComponent<IgnoreSave>();
	texty.AddComponent<IgnorePhysics>();
	return texty;
}

bool WorldSystem::load_saved_level()
{
	// Check if user has saved level progress
	const std::string saved_file_path = saved_path(yaml_file(SAVE_FILE_NAME));
	if (!Util::file_exists(saved_file_path))
	{
		return false;
	}

	// Load the saved level file
	printf("Found saved data at '%s'\n", saved_file_path.c_str());
	load_level(saved_file_path, 0);
	return true;
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
		if (levels[i] == ActiveScene->m_Id) {
			return i;
		}
	}
	return 0;
}

void WorldSystem::draw_projected_path(ECS_ENTT::Entity slingBro, vec2 mouse_pos){
	RemoveAllEntitiesWithComponent<ProjectedPath>();
	auto motion = slingBro.GetComponent<Motion>();
	auto broPosition = vec3(motion.position.x, motion.position.y, motion.position.z);
	auto broSlingMotion = slingBro.GetComponent<SlingMotion>();

	glm::vec2 dispVecFromBro = WorldSystem::getDispVecFromSource(glm::vec2(mouse_pos), glm::vec3(broPosition));
	vec2 dragDirLoop = -dispVecFromBro;
	vec2 dragMagnitude = broSlingMotion.magnitude * length(dragDirLoop);
	dragDirLoop.x *= dragMagnitude.x;
	dragDirLoop.y *= dragMagnitude.y;
	vec3 broVel = vec3(dragDirLoop, 0.0) / 1000.f;

	if (glm::length(broVel) > MAX_VELOCITY)
		broVel = glm::normalize(broVel) * MAX_VELOCITY;

	float gravitational_constant = slingBro.GetComponent<Gravity>().gravitational_constant;

	float pseudo_elapsed_ms = 50;
	float scale = 15;
	float maxY = motion.position.y + motion.scale.y / 2; // position of the ground below slingbro
	for (int i = 0; i < 30; i++) // 30 stars as path
	{
		float step_seconds = (pseudo_elapsed_ms / 1000.0f);

		broVel.y += gravitational_constant * step_seconds;
		broVel.x -= broVel.x * step_seconds * HORIZONTAL_FRICTION_MAGNITUDE;
		broPosition += broVel * step_seconds;
		scale += 3; // path of increasing star sizes
		if (broPosition.y < maxY) // only draw if the point is above slingbro position
		{
			ProjectedPath::createProjectedPoint(broPosition, scale, GameScene);
		}
		pseudo_elapsed_ms += 5; // space stars further apart as it moves further away from bro.
	}

	auto& turn = get_current_player().GetComponent<Turn>();
	turn.mouse_pos = mouse_pos;
}

void WorldSystem::attach(std::function<void(ECS_ENTT::Scene* scene)> fn)
{
	callbacks.push_back(fn);
}

void WorldSystem::runWeatherCallbacks()
{
	for (std::function<void(ECS_ENTT::Scene* scene)> fn : callbacks) {
		// run the callback functions
		fn(ActiveScene);
	}
}
