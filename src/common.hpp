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

// Size of all entities
static const unsigned int SPRITE_SCALE = 100;

// Simple utility functions to avoid mistyping directory name
inline std::string data_path() { return "data"; };
inline std::string shader_path(const std::string& name) { return data_path() + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) { return data_path() + "/textures/" + name; };
inline std::string audio_path(const std::string& name) { return data_path() + "/audio/" + name; };
inline std::string mesh_path(const std::string& name) { return data_path() + "/meshes/" + name; };
inline std::string levels_path(const std::string& name) { return data_path() + "/levels/" + name; };
inline std::string create_path(const std::string& name) { return data_path() + "/create/" + name; };
inline std::string saved_path(const std::string& name) { return data_path() + "/saved/" + name; };

// Append .yaml extension to the end of the given file name
inline std::string yaml_file(const std::string& name) { return name + ".yaml"; };

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
};

// AI component that stores the behavior tree for an entity
struct AI {
	BehaviorTree::Node* behavior_tree = NULL;
};

struct SlingMotion {
	bool canClick = true;
	bool isClicked = false;
	float magnitude = 0.f;
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

struct MaxMin {
	float xMax;
	float xMin;
	float yMax;
	float yMin;
};
