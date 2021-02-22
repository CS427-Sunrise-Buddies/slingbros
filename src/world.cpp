// Header
#include "world.hpp"
#include "physics.hpp"
#include "debug.hpp"
//#include "pebbles.hpp"
#include "render_components.hpp"

#include <glm/ext/matrix_transform.hpp>

// stlib
#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>
#include <entities/ground_tile.hpp>

// Game configuration
const size_t MAX_TURTLES = 15;
const size_t MAX_FISH = 5;
const size_t TURTLE_DELAY_MS = 2000;
const size_t FISH_DELAY_MS = 5000;
const float TURTLE_SPEED = 100.0f;
const float FISH_SPEED = 200.0f;

// For this barebones template, this is just the main scene (Scenes are essentially just entity containers)
ECS_ENTT::Scene* WorldSystem::GameScene = nullptr;

Camera* WorldSystem::ActiveCamera = nullptr;

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
	WorldSystem::GameScene = new ECS_ENTT::Scene({ 1000.f, 1000.f });

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

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr)
		throw std::runtime_error("Failed to load sounds make sure the data directory is present: " +
								 audio_path("music.wav") +
								 audio_path("salmon_dead.wav") +
								 audio_path("salmon_eat.wav"));

}

// Update our game world
void WorldSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// TODO Removing out of screen entities (for the appropriate entities like projectiles, for example

	// Processing the salmon state
	assert(GameScene->m_Registry.view<ScreenState>().size() <= 1);
	auto screenStateEntitiesView = GameScene->m_Registry.view<ScreenState>();
	for (auto entity : screenStateEntitiesView)
	{
		auto& screenComponent = screenStateEntitiesView.get<ScreenState>(entity);

		// TODO using EnTT
		//for (auto entity : ECS::registry<DeathTimer>.entities)
		//{
		//	// Progress timer
		//	auto& counter = ECS::registry<DeathTimer>.get(entity);
		//	counter.counter_ms -= elapsed_ms;

		//	// Reduce window brightness if any of the present salmons is dying
		//	screenComponent.darken_screen_factor = 1 - counter.counter_ms / 3000.f;

		//	// Restart the game once the death timer expired
		//	if (counter.counter_ms < 0)
		//	{
		//		ECS::registry<DeathTimer>.remove(entity);
		//		screenComponent.darken_screen_factor = 0;
		//		restart();
		//		return;
		//	}
		//}
	}

	// !!! TODO A1: update LightUp timers and remove if time drops below zero, similar to the DeathTimer
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO using EnTT
	//for (auto entity : ECS::registry<LightUp>.entities)
	//{
	//	// Progress timer
	//	auto& lightUpComponent = ECS::registry<LightUp>.get(entity);
	//	lightUpComponent.counter_ms -= elapsed_ms;

	//	// Restart the game once the death timer expired
	//	if (lightUpComponent.counter_ms < 0)
	//		ECS::registry<LightUp>.remove(entity);
	//}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

template<typename ComponentType>
void WorldSystem::RemoveAllEntitiesWithComponent()
{
	GameScene->m_Registry.view<ComponentType>().each([](const auto entityID, auto &&...) {
			GameScene->m_Registry.destroy(entityID);
		});
}

// Reset the world state to its initial state
void WorldSystem::restart() // notes: like Game::init
{
	// TODO using EnTT
	// Debugging for memory/component leaks
	//ECS::ContainerInterface::list_all_components();
	std::cout << "Restarting\n";

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	//while (ECS::registry<Motion>.entities.size()>0)
	//	ECS::ContainerInterface::remove_all_components_of(ECS::registry<Motion>.entities.back());

	//// Debugging for memory/component leaks
	//ECS::ContainerInterface::list_all_components();

	// Remove all old entities in the scene
	if(test_bro)
		GameScene->DestroyEntity(test_bro);
	if (test_enemy)
		GameScene->DestroyEntity(test_enemy);
	RemoveAllEntitiesWithComponent<Projectile>();
	RemoveAllEntitiesWithComponent<Wall>();

	// Create a new bro
	test_bro = SlingBro::createSlingBro({ 500, 300, 0 }, GameScene);
	test_bro.AddComponent<Gravity>();
	test_enemy = BasicEnemy::createBasicEnemy({ 200, 200, 0 }, GameScene);

	// temporary walls
	float wall_displacement = 30;
	for (int i = -wall_displacement; i < GameScene->m_Size.x+wall_displacement; i += 100)
	{
		ECS_ENTT::Entity top = GroundTile::createGroundTile({ i, -wall_displacement, 0 }, GameScene); // top wall
		top.AddComponent<Wall>();
		auto bottom = GroundTile::createGroundTile({ i, GameScene->m_Size.y+wall_displacement, 0 }, GameScene); // bottom wall
		bottom.AddComponent<Wall>();
	}
	for (int i = -wall_displacement; i < GameScene->m_Size.y+wall_displacement; i += 100)
	{
		auto left = GroundTile::createGroundTile({ -wall_displacement, i, 0 }, GameScene); // left wall
		left.AddComponent<Wall>();
		auto right = GroundTile::createGroundTile({ GameScene->m_Size.x+wall_displacement, i, 0 }, GameScene); // right wall
		right.AddComponent<Wall>();
	}



	// !! TODO A3: Enable static pebbles on the ground
	/*
	// Create pebbles on the floor
	for (int i = 0; i < 20; i++)
	{
		int w, h;
		glfwGetWindowSize(m_window, &w, &h);
		float radius = 30 * (m_dist(m_rng) + 0.3f); // range 0.3 .. 1.3
		Pebble::createPebble({ m_dist(m_rng) * w, h - m_dist(m_rng) * 20 }, { radius, radius });
	}
	*/
}


// Collisions between wall and non-wall entities - callback function, listening to PhysicsSystem::Collisions
// example of observer pattern
void WorldSystem::collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall)
{
	if (!hit_wall && entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<Projectile>()) {
		++points;
		GameScene->m_Registry.destroy(entity_j);
	}
}

/*
// Compute collisions between entities
void WorldSystem::handle_collisions()
{
	// Loop over all collisions detected by the physics system
	auto registry = GameScene->m_Registry.view<PhysicsSystem::Collision>();

    for (const entt::entity entity : registry)
	{
		// The entity and its collider
		auto entity = registry.entities[i];
		auto entity_other = registry.components[i].other;

		// if collision, bounce
		if (ECS::registry<Salmon>.has(entity))
		{
			// Handled collisions here

			// EXAMPLE
			// Checking Salmon - Fish collisions
			//if (ECS::registry<Fish>.has(entity_other))
			//{

			//}
		}
	}

	// Remove all collisions from this simulation step
	ECS::registry<PhysicsSystem::Collision>.clear();
}
*/

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
// TODO A1: check out https://www.glfw.org/docs/3.3/input_guide.html
void WorldSystem::on_key(int key, int, int action, int mod)
{
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart();
	}

	// Debugging
	if (key == GLFW_KEY_D)
		DebugSystem::in_debug_mode = (action != GLFW_RELEASE);

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
	{
		current_speed -= 0.1f;
		std::cout << "Current speed = " << current_speed << std::endl;
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
	{
		current_speed += 0.1f;
		std::cout << "Current speed = " << current_speed << std::endl;
	}
	current_speed = std::max(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_pos)
{

	//if (!ECS::registry<DeathTimer>.has(player_salmon))
	// tiny_ecs way above, EnTT below:
	if (!test_bro.HasComponent<DeathTimer>())
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// xpos and ypos are relative to the top-left of the window, the salmon's
		// default facing direction is (1, 0)
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		//auto& salmonMotionComponent = ECS::registry<Motion>.get(player_salmon);
		// tiny_ecs way above, EnTT below:
		auto& m_slingbro = test_bro.GetComponent<Motion>();
		vec2 disp = mouse_pos - vec2(m_slingbro.position);
		float angle = atan2(disp.y, disp.x);
		m_slingbro.angle = angle;
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	auto& testBroMotionComponent = test_bro.GetComponent<Motion>();
	glm::mat4 testBroModelMatrix = glm::translate(glm::mat4(1), testBroMotionComponent.position);
	glm::vec4 testBroPosClipSpace = projMatrix * viewMatrix * testBroModelMatrix * glm::vec4(1.0f);
	glm::vec2 testBroNDCS = glm::vec2(testBroPosClipSpace.x, testBroPosClipSpace.y) /
							testBroPosClipSpace.w; // why is x in [-1,1] and y in [1,-1] ????
	// TODO replace hardcoded window width, height in the following line
	glm::vec2 testBroPosScreenSpace = glm::vec2(((testBroNDCS.x + 1.0f) / 2.0f) * 1200,
			800 - ((testBroNDCS.y + 1.0f) / 2.0f) * 800);
	glm::vec2 dispVecFromTestBro = mouse_pos - testBroPosScreenSpace;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool& isBroClicked = test_bro.GetComponent<SlingMotion>().isClicked;
	vec2& dragDir = test_bro.GetComponent<SlingMotion>().direction;
	float& dragMagnitude = test_bro.GetComponent<SlingMotion>().magnitude;
	vec3& test_broMotion = test_bro.GetComponent<Motion>().velocity;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (abs(dispVecFromTestBro.x) < 100 && abs(dispVecFromTestBro.y) < 100)
		{
			isBroClicked = true;
		}

	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && isBroClicked)
	{
		isBroClicked = false;
		dragDir = testBroPosScreenSpace - mouse_pos;
		dragMagnitude = sqrt(dragDir.x * dragDir.x + dragDir.y * dragDir.y);
		test_broMotion = vec3(dragMagnitude * 2 * dragDir, 0.0) / 1000.f;
		std::cout << "vel: " << test_broMotion.x << " " << test_broMotion.y << std::endl;
		// TODO: Set max velocity for bro
		test_bro.GetComponent<Motion>().velocity = glm::vec3(test_broMotion.x, test_broMotion.y, 0.0f);
	}
}


void WorldSystem::HandlePlayerMovement(float deltaTime)
{
	// Move slingbro if alive
	//if (!ECS::registry<DeathTimer>.has(player_salmon))
	// tiny_ecs way above, EnTT below:
	if (!test_bro.HasComponent<DeathTimer>())
	{
		printf("Ain't nothin yet\n");
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

	camera->SetPosition(cameraPosition);
	camera->SetRotationZ(cameraRotationZ);

	// Testing 3D perspective
	// Press T and G to move the test_bro along z axis
	if (IsKeyPressed(GLFW_KEY_T))
		test_bro.GetComponent<Motion>().position.z -= 10.0f * deltaTime / 100.0f;
	if (IsKeyPressed(GLFW_KEY_G))
		test_bro.GetComponent<Motion>().position.z += 10.0f * deltaTime / 100.0f;
}
