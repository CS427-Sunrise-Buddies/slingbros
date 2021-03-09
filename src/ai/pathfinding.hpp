#pragma once

#include <common.hpp>
#include <queue>
#include "behavioral_tree.hpp"
#include "Entity.h"

const size_t MOVEMENT_DELAY = 100;
// TODO: allow diagonal grid traversal?
static const vec2 BFS_DIRECTIONS[] = {
		vec2(-1.0f, 0.0f),
		vec2(1.0f, 0.0f),
		vec2(0.0f, 1.0f),
		vec2(0.0f, -1.0f),
};

class MoveToGoal : public BehaviorTree::Node
{
public:
	MoveToGoal();

private:
	void init(ECS_ENTT::Entity e) override;

	BehaviorTree::State process(ECS_ENTT::Entity e) override;
	bool isGoalTile(std::string s);
	bool isTile(std::string s);

	std::queue<vec2> m_Path;
	size_t m_IterationNum;
	bool m_GoalReached;
};
