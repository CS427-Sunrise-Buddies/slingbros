#include "ai.hpp"
#include "world.hpp"

void AISystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	(void)window_size_in_game_units; // placeholder to silence unused warning until implemented

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
}
