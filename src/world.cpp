// Header
#include "world.hpp"
#include "physics.hpp"
#include "debug.hpp"
#include "render_components.hpp"

#include <glm/ext/matrix_transform.hpp>

// stlib
#include <string.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>
#include <entities/powerup.hpp>
#include <entities/speed_powerup.hpp>
#include "entities/slingbro.hpp"
#include "entities/projectile.hpp"
#include <entities/button.hpp>
#include <entities/help_screen.hpp>
#include <loader/level_manager.hpp>
#include <animation.hpp>
#include <entities/goal_tile.hpp>
#include <entities/snail_enemy.hpp>

// Game configuration
ECS_ENTT::Scene* WorldSystem::ActiveScene = nullptr;

// For this barebones template, this is just the main scene (Scenes are essentially just entity containers)
ECS_ENTT::Scene* WorldSystem::GameScene = nullptr;
ECS_ENTT::Scene* WorldSystem::MenuScene = nullptr;

Camera* WorldSystem::ActiveCamera = nullptr;

bool WorldSystem::helpScreenToggle = false;

static const std::string levels[7] = {"Tutorial", "StairLevel", "HighUp", "spiral", "symmetry", "danceforme", "hell"};
int levelNumber = 0;

bool isLoadNextLevel = false;

bool WorldSystem::getIsLoadNextLevel() {
	return isLoadNextLevel;
}

void WorldSystem::setIsLoadNextLevel(bool b) {
	isLoadNextLevel = b;
}

// Create the fish world
// Note, this has a lot of OpenGL specific things, could be moved to the renderer; but it also defines the callbacks to the mouse and keyboard. That is why it is called here.
WorldSystem::WorldSystem(ivec2 window_size_px) :
		points(0)
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

	// Initialize the only scene we have right now
	WorldSystem::MenuScene = new ECS_ENTT::Scene("Menu", { 10, 10 });
	WorldSystem::GameScene = new ECS_ENTT::Scene("Default", { 10, 10 });

	// Initialize Game on the Menu scene
	WorldSystem::ActiveScene = MenuScene;
	
	// Initialize the only camera we have right now
	WorldSystem::ActiveCamera = new Camera(); // defaults to orthographic projection
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
	Mix_CloseAudio();

	// TODO using EnTT
	// Destroy all created components
	//ECS::ContainerInterface::clear_all_components();

	// Free memory of all of the game's scenes
	delete (GameScene);
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

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("salmon_eat.wav").c_str());

	if (background_music == nullptr ||salmon_dead_sound == nullptr || salmon_eat_sound == nullptr)
		throw std::runtime_error("Failed to load sounds make sure the data directory is present: " +
								 audio_path("music.wav") +
								 audio_path("salmon_dead.wav") +
								 audio_path("salmon_eat.wav"));

}


ECS_ENTT::Scene* WorldSystem::MenuInit(vec2 window_size_in_game_units)
{
	ECS_ENTT::Scene* menuScene = WorldSystem::MenuScene;
	
	vec2 startButtonPos = vec2(window_size_in_game_units.x/2.f-150.f, window_size_in_game_units.y/2.f-200.f);
	vec2 quitButtonPos = vec2(window_size_in_game_units.x/2.f-150.f, window_size_in_game_units.y/2.f);

	ECS_ENTT::Entity startGameButtonEntity = Button::createButton(startButtonPos, vec2(300.f,100.f), "button_start", ActiveScene);
	auto& startComponent = startGameButtonEntity.AddComponent<ClickableText>();
	startComponent.functionName = "play";
	
	ECS_ENTT::Entity quitGameButtonEntity = Button::createButton(quitButtonPos,  vec2(300.f,100.f), "button_quit", ActiveScene);
	auto& quitComponent = quitGameButtonEntity.AddComponent<ClickableText>();
	quitComponent.functionName = "quit";
	
	return menuScene;
}



// Update our game world
ECS_ENTT::Scene* WorldSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// TODO Removing out of screen entities (for the appropriate entities like projectiles, for example)

	// TODO remove this?
	// Processing the salmon state
	assert(ActiveScene->m_Registry.view<ScreenState>().size() <= 1);
	auto screenStateEntitiesView = ActiveScene->m_Registry.view<ScreenState>();
	for (auto entity : screenStateEntitiesView)
		auto& screenComponent = screenStateEntitiesView.get<ScreenState>(entity);
	return ActiveScene;
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

void WorldSystem::load_next_level()
{
	std::cout << "loading next level\n";

	// Remove all old entities in the scene
	remove_all_entities();

	WorldSystem::load_level();
	ActiveScene = GameScene;

	// Point camera at the bro
	auto slingbro = get_current_player();
	auto p_slingbro = slingbro.GetComponent<Motion>().position;
	auto p_camera = ActiveCamera->GetPosition();
	ActiveCamera->SetPosition(vec3(p_slingbro.x, p_slingbro.y, p_camera.z));
}

// Reset the world state to its initial state
void WorldSystem::restart() // notes: like Game::init
{
	std::cout << "Restarting\n";

	// Remove all old entities in the scene
	remove_all_entities();

	// Load the scene
	// TODO: [BROS-28] UX Help
	WorldSystem::reload_level();

	// Point camera at the bro
	auto slingbro = get_current_player();
	auto p_slingbro = slingbro.GetComponent<Motion>().position;
	auto p_camera = ActiveCamera->GetPosition();
	ActiveCamera->SetPosition(vec3(p_slingbro.x, p_slingbro.y, p_camera.z));
}

void powerup_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, ECS_ENTT::Scene* gameScene) {
	if (entity_j.HasComponent<SpeedPowerUp>()) {
		SpeedPowerUp& speed_power_up = entity_j.GetComponent<SpeedPowerUp>();
		speed_power_up.applyPowerUp(entity_i);
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

	if (!hit_wall && entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<Projectile>()) {
		++points;
		ActiveScene->m_Registry.destroy(entity_j);
	} else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<PowerUp>()) {
		powerup_collision_listener(entity_i, entity_j, ActiveScene);
	}
	else if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<GoalTile>())
	{
		levelNumber++;
		if (levelNumber < sizeof(levels)/sizeof(levels[0]))
		{
			isLoadNextLevel = true;
		}
	}
	else if (entity_i.HasComponent<SnailEnemy>() && entity_j.HasComponent<GoalTile>())
	{
		isLoadNextLevel = true;
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
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R && ActiveScene != MenuScene)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart();
		ActiveScene = GameScene;
	}
	
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE && ActiveScene != MenuScene)
	{
		ActiveScene = MenuScene;
		restart();
	}
	
	if (action == GLFW_RELEASE && key == GLFW_KEY_H)
	{
		if (ActiveScene->m_Registry.view<HelpScreen>().size() > 0)
		{
			WorldSystem::RemoveAllEntitiesWithComponent<HelpScreen>();
			WorldSystem::helpScreenToggle = false;
			for (auto ent : ActiveScene->m_Registry.view<SlingMotion>())
			{
				SlingMotion& slingComponent = ActiveScene->m_Registry.get<SlingMotion>(ent);
				slingComponent.canClick = true;
			}
			
			for (auto ent : ActiveScene->m_Registry.view<ClickableText>())
			{
				ClickableText& buttonComponent = ActiveScene->m_Registry.get<ClickableText>(ent);
				buttonComponent.canClick = true;
			}
		}
		else
		{
			ECS_ENTT::Entity helpScreenEntity = HelpScreen::createHelpScreen(ActiveScene);
			WorldSystem::helpScreenToggle = true;
			
			for (auto ent : ActiveScene->m_Registry.view<SlingMotion>())
			{
				SlingMotion& slingComponent = ActiveScene->m_Registry.get<SlingMotion>(ent);
				slingComponent.canClick = false;
			}
			
			for (auto ent : ActiveScene->m_Registry.view<ClickableText>())
			{
				ClickableText& buttonComponent = ActiveScene->m_Registry.get<ClickableText>(ent);
				buttonComponent.canClick = false;
			}
		}
	}
	

	// Debugging
	if (key == GLFW_KEY_Z)
	{
		DebugSystem::in_debug_mode = (action != GLFW_RELEASE);
	}

	// Saving
	if (action == GLFW_RELEASE && key == GLFW_KEY_ENTER)
	{
		LevelManager::save_level(GameScene);
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_pos)
{
	auto slingbro = get_current_player();

	if (!slingbro.HasComponent<DeathTimer>())
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// xpos and ypos are relative to the top-left of the window
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		auto& m_slingbro = slingbro.GetComponent<Motion>();
		vec2 disp = mouse_pos - vec2(m_slingbro.position);
		float angle = atan2(disp.y, disp.x);
		m_slingbro.angle = angle;
	}

	for(auto entity : WorldSystem::ActiveScene->m_Registry.view<ClickableText>())
	{
		ClickableText& clickableText = WorldSystem::ActiveScene->m_Registry.get<ClickableText>(entity);
		Motion& textComponent = WorldSystem::ActiveScene->m_Registry.get<Motion>(entity);
		//WorldSystem::ActiveScene->m_Registry.get<Text>(entity);
		// TODO: can't invert y properly until scaling is fixed
		
		glm::mat4 viewMatrix = ActiveCamera->GetViewMatrix();
		glm::mat4 projMatrix = ActiveCamera->GetProjectionMatrix();
		glm::mat4 textModelMatrix = glm::translate(glm::mat4(1), textComponent.position);
		glm::vec4 textPosClipSpace = projMatrix * viewMatrix * textModelMatrix * glm::vec4(1.0f);
		glm::vec2 textNDCS = glm::vec2(textPosClipSpace.x, textPosClipSpace.y) /
		textPosClipSpace.w;
		// TODO replace hardcoded window width, height in the following line
		glm::vec2 textPosScreenSpace = glm::vec2(((textNDCS.x + 1.0f) / 2.0f) * 1200,
				800 - ((textNDCS.y + 1.0f) / 2.0f) * 800);
		glm::vec2 dispVecFromText = mouse_pos - textPosScreenSpace;

		float scaleX = abs(textComponent.scale.x / 2.f) - 27.f;
		float scaleY = abs(textComponent.scale.y / 2.f) - 10.f;
		if (dispVecFromText.x >= -scaleX && dispVecFromText.x <= scaleX && dispVecFromText.y >= -scaleY && dispVecFromText.y <= scaleY && clickableText.canClick)
		{
			clickableText.isHoveredOver = true;
		}
		else
		{
			clickableText.isHoveredOver = false;
		}
	}
	
	// TODO:
	/*
	- find bounding box/calculate centre of character
	- for visual component, track isClicked to activate, and use bro_position - mouse pos to get vector
	- make helper function for world coordinate conversion (where do it go?)
	 **/

}

// Don't think it's needed right now
void WorldSystem::on_mouse_click(int button, int action, int mods)
{
	double mouseXPos, mouseYPos;
	glfwGetCursorPos(window, &mouseXPos, &mouseYPos);
	vec2 mouse_pos = vec2(mouseXPos, mouseYPos);

	glm::mat4 viewMatrix = ActiveCamera->GetViewMatrix();
	glm::mat4 projMatrix = ActiveCamera->GetProjectionMatrix();

	auto slingbro = get_current_player();

	// Convert character's position from local/world coords to screen coords
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	auto& broMotion = slingbro.GetComponent<Motion>();
	glm::mat4 broModelMatrix = glm::translate(glm::mat4(1), broMotion.position);
	glm::vec4 broPosClipSpace = projMatrix * viewMatrix * broModelMatrix * glm::vec4(1.0f);
	glm::vec2 broNDCS = glm::vec2(broPosClipSpace.x, broPosClipSpace.y) / broPosClipSpace.w;
	glm::vec2 broPosScreenSpace = glm::vec2(((broNDCS.x + 1.0f) / 2.0f) * WINDOW_SIZE_IN_PX.x,
												WINDOW_SIZE_IN_PX.y - ((broNDCS.y + 1.0f) / 2.0f) * WINDOW_SIZE_IN_PX.y);
	glm::vec2 dispVecFromBro = mouse_pos - broPosScreenSpace;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	auto& broSlingMotion = slingbro.GetComponent<SlingMotion>();
	bool& canClick = broSlingMotion.canClick;
	bool& isBroClicked = broSlingMotion.isClicked;
	vec2& dragDir = broSlingMotion.direction;
	float& dragMagnitude = broSlingMotion.magnitude;
	vec3& broVelocity = broMotion.velocity;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (abs(dispVecFromBro.x) < broMotion.scale.x && abs(dispVecFromBro.y) < broMotion.scale.y && canClick)
		{
			isBroClicked = true;
		}
		
		for(auto entity : WorldSystem::ActiveScene->m_Registry.view<ClickableText>())
		{
			ClickableText& clickableText = ActiveScene->m_Registry.get<ClickableText>(entity);
			if (clickableText.isHoveredOver && clickableText.isClicked == false)
			{
				clickableText.isClicked = true;
				if (clickableText.functionName == "play")
				{
					printf("lets go to level 1..\n");
					clickableText.isClicked = false;
					ActiveScene = GameScene;
				}
				else if (clickableText.functionName == "quit")
				{
					printf("exit");
					clickableText.isClicked = false;
					exit(0);
				}
			}
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && isBroClicked)
	{
		isBroClicked = false;
		dragDir = broPosScreenSpace - mouse_pos;
		dragMagnitude = sqrt(dragDir.x * dragDir.x + dragDir.y * dragDir.y) * 10;
		broVelocity = vec3(dragMagnitude * dragDir, 0.0) / 1000.f;
		std::cout << "vel: " << broVelocity.x << " " << broVelocity.y << std::endl;

		// setting max speed for the bro, maybe load the velocities when we have the level loader
		broVelocity.x = glm::clamp(broVelocity.x, -MAX_VELOCITY, MAX_VELOCITY);
		broVelocity.y = glm::clamp(broVelocity.y, -MAX_VELOCITY, MAX_VELOCITY);

		broMotion.velocity = glm::vec3(broVelocity.x, broVelocity.y, 0.0f);
	}
}

void WorldSystem::HandleCameraMovement(Camera* camera, float deltaTime)
{
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
		camera->SetPerspective(glm::radians(80.0f), 0.01f, 2000.0f);
}

ECS_ENTT::Entity WorldSystem::get_current_player() {
	// TODO: Make it turn-based multiplayer [BROS-34] Delay-agnostic design
	assert(GameScene->m_Registry.view<SlingBro>().size() >= 1);
	auto slingbro_id = GameScene->m_Registry.view<SlingBro>().back();
	return ECS_ENTT::Entity(slingbro_id, WorldSystem::GameScene);
}

void WorldSystem::load_level()
{

	// If no levels exist, workaround is to create it again
	// TODO: @Amy modify line below to the first level in the levels array
	const std::string level_file_path = levels_path(yaml_file(levels[levelNumber]));
	if (!file_exists(level_file_path))
	{
		printf("Creating level to '%s'\n", level_file_path.c_str());
		// TODO: @Amy modify line below to the first level in the levels array
		LevelManager::create_level(yaml_file(levels[levelNumber]));
	}
	// Load the first level
	printf("Loading level from '%s'\n", level_file_path.c_str());
	WorldSystem::GameScene = LevelManager::load_level(level_file_path);
}

void WorldSystem::reload_level()
{

	// User has saved level progress, so load the saved level file
	const std::string saved_file_path = saved_path(yaml_file(SAVE_FILE_NAME));
	if (file_exists(saved_file_path))
	{
		printf("Loading from saved file '%s'\n", saved_file_path.c_str());
		WorldSystem::GameScene = LevelManager::load_level(saved_file_path);
		return;
	}
	load_level();
}

bool WorldSystem::file_exists(const std::string& file_path)
{
	// Try to open the file
	std::ifstream fin(file_path);

	// To exist or not to exist
	return !!fin;
}
