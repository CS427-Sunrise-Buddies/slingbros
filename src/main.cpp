#define GL3W_IMPLEMENTATION

#include <gl3w.h>

// stlib
#include <chrono>
#include <iostream>

// internal
#include "common.hpp"
#include "world.hpp"
#include "render.hpp"
#include "physics.hpp"
#include "ai.hpp"
#include "animation.hpp"
#include "debug.hpp"

#include "Entity.h"
#include "Camera.h"

using Clock = std::chrono::high_resolution_clock;

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
	WorldSystem world(WINDOW_SIZE_IN_PX);
	RenderSystem renderer(*world.window);
	PhysicsSystem physics;
	AnimationSystem animSystem;
	AISystem ai;

	// Initialize camera properties
	Camera* activeCamera = WorldSystem::GetActiveCamera();
	activeCamera->SetViewportSize(WINDOW_SIZE_IN_PX.x, WINDOW_SIZE_IN_PX.y);
	activeCamera->SetPosition(glm::vec3(400, 300, 600));
	activeCamera->SetRotationZ(0);
	activeCamera->SetPerspective(glm::radians(80.0f), 0.01f, 2000.0f);

	// Observer Pattern: attach collision listeners 
	physics.attach([&world](ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall) {
		world.collision_listener(entity_i, entity_j, hit_wall);
	});
	physics.attach([&animSystem](ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall) {
		animSystem.collision_listener(entity_i, entity_j, hit_wall);
	});

	// Set all states to default
	world.restart();
	auto time = Clock::now();
	
	WorldSystem::ActiveScene = WorldSystem::MenuInit(WINDOW_SIZE_IN_GAME_UNITS);

	auto freeze_time = DebugSystem::freeze_delay_ms;
	// Variable timestep loop
	while (!world.is_over())
	{
		glEnable(GL_BLEND);
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms = min(30.f, static_cast<float>((std::chrono::duration_cast<std::chrono::microseconds>(now - time)).count()) / 1000.f);
		time = now;

		world.HandleCameraMovement(activeCamera, elapsed_ms);
		
		if (!DebugSystem::in_freeze_mode && !WorldSystem::helpScreenToggle) {
			DebugSystem::clearDebugComponents();
			ai.step(elapsed_ms, WINDOW_SIZE_IN_GAME_UNITS);
			WorldSystem::ActiveScene = world.step(elapsed_ms, WINDOW_SIZE_IN_GAME_UNITS);
			if (world.getIsLoadNextLevel())
			{
				world.load_next_level();
				world.setIsLoadNextLevel(false);
				continue;
			}
			physics.step(elapsed_ms, WINDOW_SIZE_IN_GAME_UNITS);
			animSystem.step(elapsed_ms, WorldSystem::ActiveScene);
		}
		else if (DebugSystem::in_freeze_mode)
		{
			freeze_time -= elapsed_ms;
			if (freeze_time <= 0) {
				DebugSystem::in_freeze_mode = false;
				freeze_time = DebugSystem::freeze_delay_ms;
			}
		}
		renderer.draw(WINDOW_SIZE_IN_GAME_UNITS, *activeCamera);
	}

	return EXIT_SUCCESS;
}
