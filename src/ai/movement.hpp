#pragma once

#include <common.hpp>
#include "behavioral_tree.hpp"

class Patrol : public BehaviorTree::Node {
public:
	Patrol(int stepsBeforeTurn);

private:
	void init(ECS_ENTT::Entity e) override;

	BehaviorTree::State process(ECS_ENTT::Entity e, float elapsed_ms) override;

	int stepsBeforeTurn;
	int n;
};

class SkyPatrol : public BehaviorTree::Node {
public:
	SkyPatrol(int stepsBeforeTurn);

private:
	void init(ECS_ENTT::Entity e) override;

	BehaviorTree::State process(ECS_ENTT::Entity e, float elapsed_ms) override;

	int stepsBeforeTurn;
	int n;
};
