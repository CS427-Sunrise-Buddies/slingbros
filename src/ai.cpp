#include "ai.hpp"
#include "world.hpp"

void AISystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	(void)window_size_in_game_units; // placeholder to silence unused warning until implemented

	if (WorldSystem::is_ai_turn) {
		trigger_ai_movement(true);
		auto aiView = WorldSystem::ActiveScene->m_Registry.view<AI>();
		for (auto entityId : aiView) {
			auto& aiComponent = aiView.get<AI>(entityId);
			ECS_ENTT::Entity entity = ECS_ENTT::Entity(entityId, WorldSystem::ActiveScene);
			assert(aiComponent.behavior_tree && "AI component behavior tree should not be null");
			BehaviorTree::State state = aiComponent.behavior_tree->process(entity);

			if (state == BehaviorTree::State::Successful || state == BehaviorTree::State::Failure) {
				aiComponent.behavior_tree->init(entity);
			}
		}
		turn_countdown -= elapsed_ms;
		if (turn_countdown < 0) {
			trigger_ai_movement(false);
			turn_countdown = AI_TURN_COUNTDOWN;
			WorldSystem::is_ai_turn = false;
		}
	}
}

void AISystem::trigger_ai_movement(bool canMove)
{
	auto motionView = WorldSystem::ActiveScene->m_Registry.view<Motion>();
	for (auto entityId : motionView) {
		ECS_ENTT::Entity entity = ECS_ENTT::Entity(entityId, WorldSystem::ActiveScene);
		if (entity.HasComponent<AI>()) {
			auto& motionComponent = motionView.get<Motion>(entityId);
			motionComponent.can_move = canMove;
		}
	}
}
