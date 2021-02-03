// Header
#include "world.hpp"
#include "physics.hpp"
#include "debug.hpp"
//#include "pebbles.hpp"
#include "render_components.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>

// Game configuration
const size_t MAX_TURTLES = 15;
const size_t MAX_FISH = 5;
const size_t TURTLE_DELAY_MS = 2000;
const size_t FISH_DELAY_MS = 5000;
const float TURTLE_SPEED = 100.0f;
const float FISH_SPEED = 200.0f;

// For this barebones template, this is just the main scene (Scenes are essentially just entity containers)
ECS_ENTT::Scene* WorldSystem::GameScene = nullptr;

// Create the fish world
// Note, this has a lot of OpenGL specific things, could be moved to the renderer; but it also defines the callbacks to the mouse and keyboard. That is why it is called here.
WorldSystem::WorldSystem(ivec2 window_size_px) :
	points(0)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());

	///////////////////////////////////////
	// Initialize GLFW
	auto glfw_err_callback = [](int error, const char* desc) { std::cerr << "OpenGL:" << error << desc << std::endl; };
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
	window = glfwCreateWindow(window_size_px.x, window_size_px.y, "Salmon Game Assignment", nullptr, nullptr);
	if (window == nullptr)
		throw std::runtime_error("Failed to glfwCreateWindow");

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

	// Playing background music indefinitely
	init_audio();
	Mix_PlayMusic(background_music, -1);
	std::cout << "Loaded music\n";

	WorldSystem::GameScene = ECS_ENTT::Scene::Create();
}

WorldSystem::~WorldSystem(){
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
	delete(GameScene);

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
		throw std::runtime_error("Failed to load sounds make sure the data directory is present: "+
			audio_path("music.wav")+
			audio_path("salmon_dead.wav")+
			audio_path("salmon_eat.wav"));

}

// Update our game world
void WorldSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// TODO using EnTT
	// Removing out of screen entities
	//auto& registry = ECS::registry<Motion>;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current upon delete)
	//for (int i = static_cast<int>(registry.components.size()) - 1; i >= 0; --i) 
	//{
	//	auto& motion = registry.components[i];
	//	if (motion.position.x + abs(motion.scale.x) < 0.f)
	//	{
	//		// Added this check to make sure the salmon does not get removed
	//		if(registry.entities[i].id != player_salmon.id)
	//			ECS::ContainerInterface::remove_all_components_of(registry.entities[i]);
	//	}
	//}

	// Testing EnTT integration is functional:
	struct TagComponent
	{
		TagComponent(const std::string& tag)
			: Tag(tag) {}

		std::string Tag;
	};

	// Can't do this since tiny_ecs defines Entity too
	//auto entityTest = GameScene->CreateEntity();

	// EXAMPLE of the above code block using EnTT:
	// Use an entity view to iterate through all entities with a motion component
	//
	auto motionEntitiesView = GameScene->m_Registry.view<Motion>();
	for (auto entity : motionEntitiesView)
	{
		auto& motionComponent = motionEntitiesView.get<Motion>(entity);
		if (motionComponent.position.x + abs(motionComponent.scale.x) < 0.0f)
			GameScene->m_Registry.destroy(entity); // Might be bad, should be marked to be destroyed outside the iteration loop 
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE PEBBLE SPAWN/UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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

// Reset the world state to its initial state
void WorldSystem::restart()
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

	// Create a new salmon
	player_salmon = Salmon::createSalmon({ 100, 200 }, GameScene);

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

// Compute collisions between entities
void WorldSystem::handle_collisions()
{
	/*
	// Loop over all collisions detected by the physics system
	auto& registry = ECS::registry<PhysicsSystem::Collision>;
	for (unsigned int i=0; i< registry.components.size(); i++)
	{
		// The entity and its collider
		auto entity = registry.entities[i];
		auto entity_other = registry.components[i].other;

		// For now, we are only interested in collisions that involve the salmon
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
	*/
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return glfwWindowShouldClose(window)>0;
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
	// Note: Moved the player Salmon movement handling to WorldSystem::HandlePlayerMovement()

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
	if (!player_salmon.HasComponent<DeathTimer>())
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// TODO A1: HANDLE SALMON ROTATION HERE
		// xpos and ypos are relative to the top-left of the window, the salmon's 
		// default facing direction is (1, 0)
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//auto& salmonMotionComponent = ECS::registry<Motion>.get(player_salmon);
		// tiny_ecs way above, EnTT below:
		auto& salmonMotionComponent = player_salmon.GetComponent<Motion>();
		glm::vec2 disp = mouse_pos - salmonMotionComponent.position;
		float angle = atan2(disp.y, disp.x);
		salmonMotionComponent.angle = angle;

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		(void)mouse_pos;
	}
}

void WorldSystem::HandlePlayerMovement(float deltaTime)
{
	// Move salmon if alive
	//if (!ECS::registry<DeathTimer>.has(player_salmon))
	// tiny_ecs way above, EnTT below:
	if (!player_salmon.HasComponent<DeathTimer>())
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// TODO A1: HANDLE SALMON MOVEMENT HERE
		// key is of 'type' GLFW_KEY_
		// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//auto& salmonMotionComponent = ECS::registry<Motion>.get(player_salmon);
		// tiny_ecs way above, EnTT below:
		auto& salmonMotionComponent = player_salmon.GetComponent<Motion>();

		float playerFishMaxSpeed = 250.0f;
		float playerFishAcceleration = 50.0f;
		float momentumSlowdownFactor = 0.4f; // lower for more fish momentum

		// WASD keys
		if (IsKeyPressed(GLFW_KEY_W))
			if (salmonMotionComponent.velocity.y > -playerFishMaxSpeed)
				salmonMotionComponent.velocity.y -= playerFishAcceleration * deltaTime / 100.0f;
		if (IsKeyPressed(GLFW_KEY_S))
			if (salmonMotionComponent.velocity.y < playerFishMaxSpeed)
				salmonMotionComponent.velocity.y += playerFishAcceleration * deltaTime / 100.0f;
		if (IsKeyPressed(GLFW_KEY_A))
			if (salmonMotionComponent.velocity.x > -playerFishMaxSpeed)
				salmonMotionComponent.velocity.x -= playerFishAcceleration * deltaTime / 100.0f;
		if (IsKeyPressed(GLFW_KEY_D))
			if (salmonMotionComponent.velocity.x < playerFishMaxSpeed)
				salmonMotionComponent.velocity.x += playerFishAcceleration * deltaTime / 100.0f;
		// Key raised handling
		if (!IsKeyPressed(GLFW_KEY_W) && !IsKeyPressed(GLFW_KEY_S))
			salmonMotionComponent.velocity.y *= min((deltaTime / momentumSlowdownFactor), 0.999f);
		if (!IsKeyPressed(GLFW_KEY_A) && !IsKeyPressed(GLFW_KEY_D))
			salmonMotionComponent.velocity.x *= min((deltaTime / momentumSlowdownFactor), 0.999f);

		// Arrow keys
		if (IsKeyPressed(GLFW_KEY_UP))
			if (salmonMotionComponent.velocity.y > -playerFishMaxSpeed)
				salmonMotionComponent.velocity.y -= playerFishAcceleration * deltaTime / 100.0f;
		if (IsKeyPressed(GLFW_KEY_DOWN))
			if (salmonMotionComponent.velocity.y < playerFishMaxSpeed)
				salmonMotionComponent.velocity.y += playerFishAcceleration * deltaTime / 100.0f;
		if (IsKeyPressed(GLFW_KEY_LEFT))
			if (salmonMotionComponent.velocity.x > -playerFishMaxSpeed)
				salmonMotionComponent.velocity.x -= playerFishAcceleration * deltaTime / 100.0f;
		if (IsKeyPressed(GLFW_KEY_RIGHT))
			if (salmonMotionComponent.velocity.x < playerFishMaxSpeed)
				salmonMotionComponent.velocity.x += playerFishAcceleration * deltaTime / 100.0f;
		// Key raised handling
		if (!IsKeyPressed(GLFW_KEY_UP) && !IsKeyPressed(GLFW_KEY_DOWN))
			salmonMotionComponent.velocity.y *= min((deltaTime / (deltaTime + momentumSlowdownFactor)), 0.999f);
		if (!IsKeyPressed(GLFW_KEY_RIGHT) && !IsKeyPressed(GLFW_KEY_RIGHT))
			salmonMotionComponent.velocity.x *= min((deltaTime / (deltaTime + momentumSlowdownFactor)), 0.999f);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
}
