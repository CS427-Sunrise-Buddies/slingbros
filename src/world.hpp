#pragma once

// internal
#include "common.hpp"

#include "Scene.h"
#include "Entity.h"
#include "Camera.h"

#include <vector>
#include <stack>
#include <random>

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_mixer.h>

static const float MIN_DRAG_LENGTH = 10.f;
static const float MAX_VELOCITY = 1250.f;
static const glm::vec3 MENU_CAMERA_POSITION = glm::vec3(WINDOW_SIZE_IN_PX / 2, 690);

static const int LEVEL_COMPLETION_REWARD = 50;

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	static ECS_ENTT::Scene* ActiveScene;
	static std::stack<ECS_ENTT::Scene*> PrevScenes;
	// Initialize the main scene (Scenes are essentially just entity containers)
	static ECS_ENTT::Scene* GameScene;
	static ECS_ENTT::Scene* MenuScene;
	static ECS_ENTT::Scene* HelpScene;

	static ECS_ENTT::Scene* MenuInit(vec2 window_size_px);
	static ECS_ENTT::Scene* HelpInit();

	static Camera* GetActiveCamera()
	{
		return ActiveScene->GetCamera();
	}

	// Number of players in the current game
	static const unsigned int num_players;

	static bool is_ai_turn;

	static bool is_in_dialogue;

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
	void powerup_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, ECS_ENTT::Scene* gameScene);

	// Handle camera movement
	void HandleCameraMovement(Camera* camera, float deltaTime);

	// Should the game be over ?
	bool is_over() const;

	bool getIsLoadNextLevel();

	void setIsLoadNextLevel(bool b);

	void load_next_level();
	
	void handleDialogue();

	glm::vec2 getDispVecFromSource(glm::vec2 mouse_pos, glm::vec3 source_pos);

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

	static bool is_game_scene();

	static bool is_menu_scene();

	static bool is_help_scene();

	static bool is_moving(ECS_ENTT::Entity e);

	static void spawn_players();

	static void point_camera_at_current_player();

	static ECS_ENTT::Entity get_current_player();

	static void current_player_effects_tick();

	static bool should_end_turn(float elapsed_ms);

	static void save_turn_point_information(int* arr);

	static void load_turn_point_information(int turnPoints[]);

	static void reward_current_player();

	static unsigned int set_next_player();

	unsigned int get_level_number();

	void load_level(const std::string& string, bool spawn_players);

	void reload_level();

	static bool file_exists(const std::string& file_path);

	static void remove_all_entities();

	template<typename ComponentType>
	void RemoveAllEntitiesWithComponent();

private:
	// Levels in the game
	std::vector<std::string> levels;

	// music references
//	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	Mix_Music* background_music;
	Mix_Chunk* yeehaw_sound;
	Mix_Chunk* poppin_click_sound;
	Mix_Chunk* transport_sound;
	Mix_Chunk* short_grass_sound;
	Mix_Chunk* shorter_grass_sound;
	Mix_Chunk* short_thud_sound;
	Mix_Chunk* power_up_sound;
	Mix_Chunk* sizzle_sound;
	Mix_Chunk* power_up_8bit_sound;
	Mix_Chunk* balloon_tap_sound;
	Mix_Chunk* snail_monster_sound;
	Mix_Chunk* squeak_sound;
	Mix_Chunk* short_monster_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
