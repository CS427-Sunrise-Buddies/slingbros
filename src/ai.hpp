#pragma once

#include <vector>

#include "common.hpp"

#include "Entity.h"

class AISystem
{
public:
	void step(float elapsed_ms, vec2 window_size_in_game_units);

private:
	void trigger_ai_movement(bool canMove);

	float turn_countdown = AI_TURN_COUNTDOWN;
};
