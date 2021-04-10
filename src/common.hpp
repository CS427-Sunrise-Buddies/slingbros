#pragma once

// stlib
#include <string>
#include <tuple>
#include <vector>
#include <stdexcept>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3

#include "ai/behavioral_tree.hpp"

using namespace glm;

// Size of all entities
static const unsigned int SPRITE_SCALE = 100;

// Note, here the window will show a width x height part of the game world, measured in px.
// You could also define a window to show 1.5 x 1 part of your game world, where the aspect ratio depends on your window size.
const ivec2 WINDOW_SIZE_IN_PX		 = { 1200, 800 };
const vec2 WINDOW_SIZE_IN_GAME_UNITS = { 1200, 800 };

// Multiplayer constants
const size_t NUM_PLAYERS_1 = 1; // Minimum number of players in the game is 1
const size_t NUM_PLAYERS_2 = 2; // Maximum number of players in the game is 2

// Turn constants
const float END_TURN_COUNTDOWN = 1000.f; // Duration to wait for player movement to end turn
const float END_TURN_VELOCITY_X = 20.f;  // Velocity in x-direction above which to reset end turn countdown
const float END_TURN_VELOCITY_Y = 200.f; // Velocity in y-direction above which to reset end turn countdown
const float AI_TURN_COUNTDOWN = 3 * END_TURN_COUNTDOWN; // Duration to wait for AI movement to end turn
const float AI_ACTION_COUNTDOWN = 500.f; // Duration to wait for AI process
const float AI_SPEED = 70.f; // Default speed of AI entities
const float PROJECTED_PATH_FADE_COUNTDOWN = 2000.f; // fading cycle of the projected path
// Camera constants
const vec3 MENU_CAMERA_POSITION = glm::vec3(WINDOW_SIZE_IN_PX / 2, 690);
const float GAME_CAMERA_PERSPECTIVE_FAR_BOUND = 4000.0f;
const float MENU_CAMERA_PERSPECTIVE_FAR_BOUND = 2000.0f;
const float MAX_CAMERA_Z_POSITION = 2000.0f;
const float MIN_CAMERA_Z_POSITION = -900.0f;

// Particle system constants
const unsigned int MAX_NUM_PARTICLES = 4000;
const float DEFAULT_PARTICLE_LIFETIME_MS = 10000.0f;
const int NUM_BEES_PER_SWARM = 140;

// Spritesheet constants
const uint SPRITE_PIXEL_WIDTH = 32;
const uint SPRITE_PIXEL_HEIGHT = 32;
const uint SPRITESHEET_PIXEL_WIDTH = 512;
const uint SPRITESHEET_PIXEL_HEIGHT = 1024;

// Physics constants
const float HORIZONTAL_FRICTION_MAGNITUDE = 0.6;
const float VELOCITY_BOUNCE_MULTIPLIER = -0.7;
const float PI = 3.14159265359;

 // Game balancing constants
const int POINTS_LOST_PER_TURN_PER_BEE_SWARM = 7;
const int POINTS_GAINED_HARVESTING_HONEY = 30;
const int POINTS_GAINED_COIN_VALUE = 10;
const int POINTS_GAINED_ON_WIN = 50;
const int POINTS_LOST_UPON_SPIKE_COLLISION = 5;
const int POINTS_LOST_UPON_ENEMY_COLLISION = 5;
const int MAX_POINTS_LOST_PER_TURN = 50;
const int POINTS_LOST_BASIC_PROJECTILE = 1;
const int POINTS_LOST_HELGE_PROJECTILE = 10;
const float BASIC_PROJECTILE_MASS = 0.02f;
const float HELGE_PROJECTILE_MASS = 5.0f;
const float MAX_PROJECTILE_KNOCKBACK_VELOCITY = 400.0f;
const float PROJECTILE_AIM_RANDOMNESS_FACTOR = 60.0f;
const float HELGE_PROJECTILE_LIFETIME_MS = 150000.0f;
const float BASIC_PROJECTILE_LIFETIME_MS = 60000.0f;

const float RAIN_HORIZONTAL_FRICTION_MAGNITUDE = 0.3;

// Audio system constant
const float AUDIO_TRIGGER_VELOCITY_Y = 200.f;

// Simple utility functions to avoid mistyping directory name
inline std::string data_path() { return "data"; };
inline std::string shader_path(const std::string& name) { return data_path() + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) { return data_path() + "/textures/" + name; };
inline std::string story_textures_path(const std::string& name) { return data_path() + "/textures/story/" + name; };
inline std::string audio_path(const std::string& name) { return data_path() + "/audio/" + name; };
inline std::string mesh_path(const std::string& name) { return data_path() + "/meshes/" + name; };
inline std::string levels_path(const std::string& name) { return data_path() + "/levels/" + name; };
inline std::string create_path(const std::string& name) { return data_path() + "/create/" + name; };
inline std::string saved_path(const std::string& name) { return data_path() + "/saved/" + name; };
inline std::string config_path(const std::string& name) { return data_path() + "/config/" + name; };

// Append extension to the end of the given file name
inline std::string yaml_file(const std::string& name) { return name + ".yaml"; };
inline std::string png_file(const std::string& name) { return name + ".png"; };

struct Util {
	static bool file_exists(const std::string& file_path);
};

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
struct Transform {
	mat4 matrix = glm::mat4(1); // start with the identity
	void scale(vec3 scale);
	void rotate(float radians, vec3 axis);
	void translate(vec3 offset);
};

// All data relevant to the shape and motion of entities
struct Motion {
	float angle = 0;
	vec3 position = vec3(0, 0, 0);
	vec3 velocity = vec3(0, 0, 0);
	vec3 scale = vec3(10, 10, 1);
	bool can_move = true;
};

// AI component that stores the behavior tree for an entity
struct AI {
	BehaviorTree::Node* behavior_tree = nullptr;
	float countdown = AI_ACTION_COUNTDOWN;
	std::optional<vec2> target = std::nullopt;
};

struct SlingMotion {
	bool canClick = true;
	bool isClicked = false;
	vec2 clickPosition = {0.f, 0.f};
	vec2 magnitude = { 10.f, 20.f };
	vec2 direction = { 0, 0 };
};

// To store all entities that can be bounced off of on collision
struct BouncyTile
{
	vec3 position = vec3(0, 0, 0);
};

// To identify square sprites, used for debugging graphics
struct Tile
{
	vec3 position = vec3(0, 0, 0);
};

// To identify dangerous hazard items.
struct Hazard
{
	vec3 position = vec3(0, 0, 0);
};

// Indicates that an entity is affected by gravity
struct Gravity
{
	float gravitational_constant = 1000.0f;
};

// Indicates that an entity has mass
struct Mass
{
	float value = 1.f;
};

// Component to make certain Text entities clickable
struct ClickableText
{
	bool isHoveredOver = false;
	bool canClick = true;
	bool isClicked = false;
	std::string functionName;
};

// Component that makes the entity be ignored by the physics system
struct IgnorePhysics
{
	uint16_t placeholder = 0;
};

// Component that makes the entity be ignored by the level saver
struct IgnoreSave
{
	uint16_t placeholder = 0;
};

struct Turn {
	// Sling order
	unsigned int order = 0;

	// Whether or not the player has used their turn
	bool slung = false;

	// Points (e.g. Colliding with projectiles decreases player score)
	int points = 0;
	int pointsLostThisTurn = 0;

	// Duration to wait to ensure player movement has sufficiently settled down
	float countdown = END_TURN_COUNTDOWN;

	float path_fade_ms = PROJECTED_PATH_FADE_COUNTDOWN;

	vec2 mouse_pos;

	void addPoints(int newPoints) {
		points += newPoints;
	}

	void subPoints(int subPoints) {
		if(pointsLostThisTurn < MAX_POINTS_LOST_PER_TURN)
			subPoints <= points ? points -= subPoints : points = 0;
		pointsLostThisTurn += subPoints;
	}
};

struct SizeChanged {
	int turnsRemaining = 0;
};

struct MassChanged {
	int turnsRemaining = 0;
};

// Value corresponds to spritesheet offset
enum BroType {
	ORANGE = 0,
	PINK = 2,
};

struct PlayerProfile {
	std::string playerName;
	BroType broType;
};

struct CollidableEnemy
{
	uint32_t placeholder = 0;
};

