#pragma once

// internal
#include "common.hpp"

#include "Scene.h"
#include "Entity.h"
#include "Camera.h"

#include <vector>
#include <random>

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_mixer.h>

static const float MAX_VELOCITY = 1000.f;

static const unsigned int NUM_PLAYERS = 1;

// Container for all our entities and game logic. Individual rendering / update is 
// deferred to the relative update() methods
class WorldSystem
{
public:
	static ECS_ENTT::Scene* ActiveScene;
	// Initialize the main scene (Scenes are essentially just entity containers)
	static ECS_ENTT::Scene* GameScene;
	static ECS_ENTT::Scene* MenuScene;

	static ECS_ENTT::Scene* MenuInit(vec2 window_size_px);
	// The active camera within the world
	static Camera* ActiveCamera;

	static Camera* GetActiveCamera()
	{
		return ActiveCamera;
	}

	static bool helpScreenToggle;


public:

	// Creates a window
	WorldSystem(ivec2 window_size_px);

	// Releases all associated resources
	~WorldSystem();

	// restart level
	void restart();

	// Steps the game ahead by ms milliseconds
	ECS_ENTT::Scene* step(float elapsed_ms, vec2 window_size_in_game_units);

	// Collision callback function
	void collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall);

	// Handle camera movement
	void HandleCameraMovement(Camera* camera, float deltaTime);

	// Should the game be over ?
	bool is_over() const;

	static bool getIsLoadNextLevel();

	void setIsLoadNextLevel(bool b);

	static void load_next_level();

	// OpenGL window handle
	GLFWwindow* window;

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);

	void on_mouse_move(vec2 mouse_pos);

	void on_mouse_click(int button, int action, int mods);

	// Input polling function
	bool IsKeyPressed(const int glfwKeycode);

	// Loads the audio
	void init_audio();

	static ECS_ENTT::Entity get_current_player();

	static void load_level();

	static void reload_level();

	static bool file_exists(const std::string& file_path);

	static void remove_all_entities();

	template<typename ComponentType>
	void RemoveAllEntitiesWithComponent();

private:
	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
