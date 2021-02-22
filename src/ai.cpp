// internal
#include "ai.hpp"
//#include "tiny_ecs.hpp"
#include "world.hpp"

void AISystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE FISH AI HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will likely want to write new functions and need to create
	// new data structures to implement a more sophisticated Fish AI. 
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	(void)window_size_in_game_units; // placeholder to silence unused warning until implemented

	auto aiView = WorldSystem::GameScene->m_Registry.view<AI>();
	for (auto entityId : aiView) {
	    auto& aiComponent = aiView.get<AI>(entityId);
	    ECS_ENTT::Entity entity = ECS_ENTT::Entity(entityId, WorldSystem::GameScene);
	    BehaviorTree::State state = aiComponent.behavior_tree->process(entity);

	    if (state == BehaviorTree::State::Successful || state == BehaviorTree::State::Failure) {
	    	aiComponent.behavior_tree->init(entity);
	    }
	}
}
