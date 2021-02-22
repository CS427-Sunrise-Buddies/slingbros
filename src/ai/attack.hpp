#pragma once

#include "behavioral_tree.hpp"

const size_t MAX_PROJECTILES = 40;
const size_t PROJECTILE_SPEED_MULTIPLIER = 100;

class AttackInRange : public BehaviorTree::Node
{
public:
	AttackInRange(float dist_th);

private:
	void init(ECS_ENTT::Entity e) override;

	BehaviorTree::State process(ECS_ENTT::Entity e) override;

	float threshold;
};

class ShootInRange : public BehaviorTree::Node
{
public:
	ShootInRange(float dist_th);

private:
	void init(ECS_ENTT::Entity e) override;

	BehaviorTree::State process(ECS_ENTT::Entity e) override;

	float threshold;
};