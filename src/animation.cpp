#include "animation.hpp"
#include <world.hpp>

void AnimationSystem::step(float elapsed_ms, ECS_ENTT::Scene* activeScene)
{
	// Cycle through all entities with an Animation component and update timers / frames
	auto animationEntitiesView = activeScene->m_Registry.view<Animation>();
	for (entt::entity entityID : animationEntitiesView)
	{
		ECS_ENTT::Entity animationEntity = ECS_ENTT::Entity(entityID, activeScene);
		Animation& animComponent = animationEntity.GetComponent<Animation>();
		animComponent.step(elapsed_ms);
	}
}

// Collisions between wall and non-wall entities - callback function, listening to PhysicsSystem::Collisions
void AnimationSystem::collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall)
{
	if (hit_wall)
	{
		if (entity_i.HasComponent<Animation>())
		{
			Animation& animComponent = entity_i.GetComponent<Animation>();
			animComponent.playCollisionAnimation(4, 1);
		}
		if (entity_j.HasComponent<Animation>())
		{
			Animation& animComponent = entity_j.GetComponent<Animation>();
			animComponent.playCollisionAnimation(4, 1);
		}
	}
}
