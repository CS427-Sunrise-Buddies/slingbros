#pragma once

// internal
#include "common.hpp"
#include "salmon.hpp"

#include "Scene.h"
#include "Entity.h"
#include "Camera.h"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// Container for all our entities and game logic. Individual rendering / update is 
// deferred to the relative update() methods
class WorldSystem
{
public:
	// Initialize the main scene (Scenes are essentially just entity containers)
	static ECS_ENTT::Scene* GameScene;

	// Creates a window
	WorldSystem(ivec2 window_size_px);

	// Releases all associated resources
	~WorldSystem();

	// restart level
	void restart();

	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Check for collisions
	void handle_collisions();

	// Handle fish movement
	void HandlePlayerMovement(float deltaTime);

	// Handle camera movement
	void HandleCameraMovement(Camera* camera, float deltaTime);

	// Should the game be over ?
	bool is_over() const;

	// OpenGL window handle
	GLFWwindow* window;

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 mouse_pos);

	// Input polling function
	bool IsKeyPressed(const int glfwKeycode);

	// Loads the audio
	void init_audio();

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Game state
	float current_speed;
	ECS_ENTT::Entity player_salmon;
	
	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
