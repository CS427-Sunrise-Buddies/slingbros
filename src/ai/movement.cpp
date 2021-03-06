#include "movement.hpp"

Patrol::Patrol(int stepsBeforeTurn) : stepsBeforeTurn(stepsBeforeTurn), n(stepsBeforeTurn)
{

}

void Patrol::init(ECS_ENTT::Entity e)
{

}

BehaviorTree::State Patrol::process(ECS_ENTT::Entity e, float elapsed_ms)
{
	auto& motion = e.GetComponent<Motion>();

	if (--n == 0) {
		motion.velocity.x *= -1;
		n = stepsBeforeTurn;
	}

	return BehaviorTree::State::Successful;
}

SkyPatrol::SkyPatrol(int stepsBeforeTurn) : stepsBeforeTurn(stepsBeforeTurn), n(stepsBeforeTurn) {

}

void SkyPatrol::init(ECS_ENTT::Entity e) {

}

BehaviorTree::State SkyPatrol::process(ECS_ENTT::Entity e, float elapsed_ms) {
	auto& motion = e.GetComponent<Motion>();

	if (--n == 0) {
		motion.velocity.y *= -1;
		n = stepsBeforeTurn;
	}

	return BehaviorTree::State::Successful;
}