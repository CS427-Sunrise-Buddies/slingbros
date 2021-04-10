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
#include "particle_system.hpp"
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
	ParticleSystem* particleSystem = ParticleSystem::GetInstance();
	AISystem ai;

	// Observer Pattern: attach collision listeners 
	physics.attach([&world](ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall) {
		world.collision_listener(entity_i, entity_j, hit_wall);
	});
	physics.attach([&animSystem](ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall) {
		animSystem.collision_listener(entity_i, entity_j, hit_wall);
	});
	physics.attach([&particleSystem](ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall) {
		particleSystem->grass_collision_listener(entity_i, entity_j, hit_wall);
		particleSystem->dirt_collision_listener(entity_i, entity_j, hit_wall);
		particleSystem->lava_block_collision_listener(entity_i, entity_j, hit_wall);
		particleSystem->beehive_collision_listener(entity_i, entity_j, hit_wall);
	});

	world.attach([&particleSystem](ECS_ENTT::Scene* scene) {
		particleSystem->weather_listener(scene);
	});

	// Set all states to default
	world.restart();
	auto time = Clock::now();
	
	ECS_ENTT::Scene* menuScene = WorldSystem::MenuInit();
	Camera* activeCamera = menuScene->GetCamera();
	
	WorldSystem::HelpInit();

	WorldSystem::FinaleInit();

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

		if (world.getIsLevelRestart()) {
			world.restart();
			world.setIsLevelRestart(false);
		}
		
		if (!DebugSystem::in_freeze_mode) {
			DebugSystem::clearDebugComponents();
			ai.step(elapsed_ms, WINDOW_SIZE_IN_GAME_UNITS);
			world.step(elapsed_ms, WINDOW_SIZE_IN_GAME_UNITS);
			activeCamera = WorldSystem::ActiveScene->GetCamera();
			particleSystem->step(elapsed_ms);
			if (world.getIsLoadNextLevel())
			{
				particleSystem->clearParticles();
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
		renderer.draw(WINDOW_SIZE_IN_GAME_UNITS, *activeCamera, particleSystem);
	}

	return EXIT_SUCCESS;
}
