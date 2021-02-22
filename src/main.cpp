#define GL3W_IMPLEMENTATION

#include <gl3w.h>

// stlib
#include <chrono>
#include <iostream>

// internal
#include "common.hpp"
#include "world.hpp"
//#include "tiny_ecs.hpp"
#include "render.hpp"
#include "physics.hpp"
#include "ai.hpp"
#include "debug.hpp"

#include "Entity.h"
#include "Camera.h"

using Clock = std::chrono::high_resolution_clock;

const ivec2 window_size_in_px		 = { 1200, 800 };
const vec2 window_size_in_game_units = { 1200, 800 };
// Note, here the window will show a width x height part of the game world, measured in px. 
// You could also define a window to show 1.5 x 1 part of your game world, where the aspect ratio depends on your window size.

struct Description
{
	std::string name;

	Description(const char* str) : name(str)
	{};
};

entt::registry registry; // todo: make main.hpp and add this there


// Entry point
int main()
{
	// Initialize the main systems
	WorldSystem world(window_size_in_px);
	RenderSystem renderer(*world.window);
	PhysicsSystem physics;
	AISystem ai;

	// Initialize camera properties
	Camera* activeCamera = WorldSystem::GetActiveCamera();
	activeCamera->SetViewportSize(window_size_in_px.x, window_size_in_px.y);
	activeCamera->SetPosition(glm::vec3(400, 300, 500));
	activeCamera->SetRotationZ(0);
	activeCamera->SetPerspective(glm::radians(80.0f), 0.01f, 1000.0f);
	// attach listener to when collisions occur
	physics.attach([&world](ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall) {
		world.collision_listener(entity_i, entity_j, hit_wall);
	});

	// Set all states to default
	world.restart();
	auto time = Clock::now();
	// Variable timestep loop
	while (!world.is_over())
	{
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms = static_cast<float>((std::chrono::duration_cast<std::chrono::microseconds>(now - time)).count()) / 1000.f;
		time = now;

		DebugSystem::clearDebugComponents();
		ai.step(elapsed_ms, window_size_in_game_units);
		world.step(elapsed_ms, window_size_in_game_units);
		world.HandleCameraMovement(activeCamera, elapsed_ms);
		physics.step(elapsed_ms, window_size_in_game_units);

		renderer.draw(window_size_in_game_units, *activeCamera);
	}

	return EXIT_SUCCESS;
}
