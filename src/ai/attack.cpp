#include "attack.hpp"
#include <world.hpp>
#include <entities/slingbro.hpp>
#include <entities/projectile.hpp>
#include <entities/helge_projectile.hpp>
#include <particle_system.hpp>

AttackInRange::AttackInRange(float dist_th) : threshold(dist_th)
{

}

void AttackInRange::init(ECS_ENTT::Entity e)
{

}

BehaviorTree::State AttackInRange::process(ECS_ENTT::Entity e, float elapsed_ms)
{
	auto slingBroView = WorldSystem::ActiveScene->m_Registry.view<SlingBro>();

	for (auto slingBroEntityId : slingBroView) {
		ECS_ENTT::Entity entity = ECS_ENTT::Entity(slingBroEntityId, WorldSystem::ActiveScene);
		auto& slingBroMotion = entity.GetComponent<Motion>();
		auto& motion = e.GetComponent<Motion>();

		double dist = distance(slingBroMotion.position, motion.position);

		if (dist < threshold) {
			motion.position.x = slingBroMotion.position.x;
			motion.position.y = slingBroMotion.position.y;
			return BehaviorTree::State::Successful;
		}
	}
	return BehaviorTree::State::Failure;
}

ShootInRange::ShootInRange(float dist_th) : threshold(dist_th)
{

}

void ShootInRange::init(ECS_ENTT::Entity e)
{

}

BehaviorTree::State ShootInRange::process(ECS_ENTT::Entity e, float elapsed_ms)
{
	auto slingBroView = WorldSystem::ActiveScene->m_Registry.view<SlingBro>();

	for (auto slingBroEntityId : slingBroView) {
		ECS_ENTT::Entity entity = ECS_ENTT::Entity(slingBroEntityId, WorldSystem::ActiveScene);
		auto slingBroMotion = entity.GetComponent<Motion>();
		auto& motion = e.GetComponent<Motion>();
		double dist = distance(slingBroMotion.position, motion.position);

		if (dist < threshold) {
			// Update countdown until next attack
			auto& ai = e.GetComponent<AI>();
			ai.countdown -= elapsed_ms;

			// Shoot periodically
			if (ai.countdown <= 0.f) {
				ECS_ENTT::Entity projectile = Projectile::createProjectile(motion.position, WorldSystem::ActiveScene);
				auto& projectileMotion = projectile.GetComponent<Motion>();

				vec3 displacement = slingBroMotion.position - motion.position;
				projectileMotion.angle = atan2(displacement.y, displacement.x) + Random::Float();

				vec3 normalizedDisplacement = normalize(displacement);
				projectileMotion.velocity.x = normalizedDisplacement.x * PROJECTILE_SPEED_MULTIPLIER + (PROJECTILE_AIM_RANDOMNESS_FACTOR * (0.5f - Random::Float()));
				projectileMotion.velocity.y = normalizedDisplacement.y * PROJECTILE_SPEED_MULTIPLIER + (PROJECTILE_AIM_RANDOMNESS_FACTOR * (0.5f - Random::Float()));

				// Reset the attack countdown
				ai.countdown = AI_ACTION_COUNTDOWN;
			}
			return BehaviorTree::State::Successful;
		}
	}
	return BehaviorTree::State::Failure;
}

HelgeShootInRange::HelgeShootInRange(float dist_th) : threshold(dist_th)
{

}

void HelgeShootInRange::init(ECS_ENTT::Entity e)
{

}

BehaviorTree::State HelgeShootInRange::process(ECS_ENTT::Entity e, float elapsed_ms) 
{
	auto slingBroView = WorldSystem::ActiveScene->m_Registry.view<SlingBro>();

	for (auto slingBroEntityId : slingBroView) {
		ECS_ENTT::Entity entity = ECS_ENTT::Entity(slingBroEntityId, WorldSystem::ActiveScene);
		auto slingBroMotion = entity.GetComponent<Motion>();
		auto& motion = e.GetComponent<Motion>();
		double dist = distance(slingBroMotion.position, motion.position);

		if (dist < threshold) {
			// Update countdown until next attack
			auto& ai = e.GetComponent<AI>();
			ai.countdown -= elapsed_ms;

			// Shoot periodically
			if (ai.countdown <= 0.f) {
				ECS_ENTT::Entity projectile = HelgeProjectile::createHelgeProjectile(motion.position, WorldSystem::ActiveScene);
				auto& projectileMotion = projectile.GetComponent<Motion>();

				vec3 displacement = slingBroMotion.position - motion.position;

				vec3 normalizedDisplacement = normalize(displacement);
				projectileMotion.velocity.x = normalizedDisplacement.x * HELGE_PROJECTILE_SPEED_MULTIPLIER;
				projectileMotion.velocity.y = normalizedDisplacement.y * HELGE_PROJECTILE_SPEED_MULTIPLIER;

				// Reset the attack countdown
				ai.countdown = 1000.f;
			}
			return BehaviorTree::State::Successful;
		}
	}
	return BehaviorTree::State::Failure;
}
