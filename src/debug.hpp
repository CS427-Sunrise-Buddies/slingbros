#pragma once

#include "common.hpp"

// Data structure for pebble-specific information
namespace DebugSystem {
	extern bool in_debug_mode;
	extern bool is_gravity_on; // todo: move this somewhere else

	// draw a red line for debugging purposes
	void createLine(vec3 position, vec3 size, ECS_ENTT::Scene *scene);

	// !!! TODO A2: implement debugging of bounding boxes
	void createBox(vec2 position, vec2 size);

	// Removes all debugging graphics in ECS, called at every iteration of the game loop
	void clearDebugComponents();
};
