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

#include <ai/behavioral_tree.hpp>

using namespace glm;
static const float PI = 3.14159265359f;

// Simple utility functions to avoid mistyping directory name
inline std::string data_path() { return "data"; };
inline std::string shader_path(const std::string& name) { return data_path() + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) { return data_path() + "/textures/" + name; };
inline std::string audio_path(const std::string& name) { return data_path() + "/audio/" + name; };
inline std::string mesh_path(const std::string& name) { return data_path() + "/meshes/" + name; };

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
	bool isClicked = false;
	float magnitude = 0.f;
	vec2 direction = { 0, 0 };
};

// to store all entities that represent a wall
struct Wall
{
	vec3 position = vec3(0, 0, 0);
};

struct Gravity
{
	float gravitational_constant = 1000.0f;
};

struct MaxMin {
	float xMax;
	float xMin;
	float yMax;
	float yMin;
};