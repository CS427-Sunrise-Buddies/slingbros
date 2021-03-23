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

// Note, here the window will show a width x height part of the game world, measured in px.
// You could also define a window to show 1.5 x 1 part of your game world, where the aspect ratio depends on your window size.
const ivec2 WINDOW_SIZE_IN_PX		 = { 1200, 800 };
const vec2 WINDOW_SIZE_IN_GAME_UNITS = { 1200, 800 };

// Multiplayer constants
const unsigned int MIN_NUM_PLAYERS = 1;  // Minimum number of players in the game
const unsigned int MAX_NUM_PLAYERS = 2;  // Maximum number of players in the game

// Turn constants
const float END_TURN_COUNTDOWN = 1000.f; // Duration to wait for player movement to end turn
const float END_TURN_VELOCITY_X = 20.f;  // Velocity in x-direction above which to reset end turn countdown
const float END_TURN_VELOCITY_Y = 200.f;  // Velocity in y-direction above which to reset end turn countdown
const float AI_TURN_COUNTDOWN = 3000.f; // Duration to wait for AI movement to end turn

// Camera constants
const float GAME_CAMERA_PERSPECTIVE_FAR_BOUND = 4000.0f;
const float MENU_CAMERA_PERSPECTIVE_FAR_BOUND = 2000.0f;
const float MAX_CAMERA_Z_POSITION = 2000.0f;
const float MIN_CAMERA_Z_POSITION = -900.0f;

// Particle system constants
const unsigned int MAX_NUM_PARTICLES = 4000;
const float DEFAULT_PARTICLE_LIFETIME_MS = 10000.0f;

// Spritesheet constants
const uint SPRITE_PIXEL_WIDTH = 32;
const uint SPRITE_PIXEL_HEIGHT = 32;
const uint SPRITESHEET_PIXEL_WIDTH = 512;
const uint SPRITESHEET_PIXEL_HEIGHT = 1024;

// Size of all entities
static const unsigned int SPRITE_SCALE = 100;

// Physics constants
const float HORIZONTAL_FRICTION_MAGNITUDE = 0.6;
const float VELOCITY_BOUNCE_MULTIPLIER = -0.7;

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
	BehaviorTree::Node* behavior_tree = NULL;
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

struct Gravity
{
	float gravitational_constant = 1000.0f;
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

	// Duration to wait to ensure player movement has sufficiently settled down
	float countdown = END_TURN_COUNTDOWN;
};

struct SizeChanged {
	int turnsRemaining = 0;
};
