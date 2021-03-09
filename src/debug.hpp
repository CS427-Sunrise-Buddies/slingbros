#pragma once

#include "common.hpp"
#include "world.hpp"

// Data structure for pebble-specific information
namespace DebugSystem {
	extern bool in_debug_mode;
	extern bool in_freeze_mode;
	extern float freeze_delay_ms;

	// draw a red line for debugging purposes
	void createLine(vec3 position, vec3 size);

	void createBox(vec3 position, vec2 bounding_box, vec3 size);

	void createCircle(vec3 position, vec3 size);

	// Removes all debugging graphics in ECS, called at every iteration of the game loop
	void clearDebugComponents();
};
