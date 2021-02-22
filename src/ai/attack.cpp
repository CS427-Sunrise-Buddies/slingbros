#include "attack.hpp"
#include <world.hpp>
#include <entities/slingbro.hpp>
#include <entities/projectile.hpp>

AttackInRange::AttackInRange(float dist_th) : threshold(dist_th)
{

}

void AttackInRange::init(ECS_ENTT::Entity e)
{

}

BehaviorTree::State AttackInRange::process(ECS_ENTT::Entity e)
{
	auto slingBroView = WorldSystem::GameScene->m_Registry.view<SlingBro>();

	for (auto slingBroEntityId : slingBroView) {
		ECS_ENTT::Entity entity = ECS_ENTT::Entity(slingBroEntityId, WorldSystem::GameScene);
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

BehaviorTree::State ShootInRange::process(ECS_ENTT::Entity e)
{
	auto slingBroView = WorldSystem::GameScene->m_Registry.view<SlingBro>();

	for (auto slingBroEntityId : slingBroView) {
		ECS_ENTT::Entity entity = ECS_ENTT::Entity(slingBroEntityId, WorldSystem::GameScene);
		auto& slingBroMotion = entity.GetComponent<Motion>();
		auto& motion = e.GetComponent<Motion>();
		double dist = distance(slingBroMotion.position, motion.position);

		if (dist < threshold) {
			if (WorldSystem::GameScene->m_Registry.view<Projectile>().size() < MAX_PROJECTILES) {
				// TODO: BROS-31 figure out what is causing slingBroMotion.position and motion.position to be corrupted when createProjectile(...) is called
				vec3 displacement = slingBroMotion.position - motion.position;
				ECS_ENTT::Entity projectile = Projectile::createProjectile(motion.position, WorldSystem::GameScene);
				auto& projectileMotion = projectile.GetComponent<Motion>();

				projectileMotion.angle = atan2(displacement.y, displacement.x);

				vec3 normalizedDisplacement = normalize(displacement);
				projectileMotion.velocity.x = normalizedDisplacement.x * PROJECTILE_SPEED_MULTIPLIER;
				projectileMotion.velocity.y = normalizedDisplacement.y * PROJECTILE_SPEED_MULTIPLIER;
			}
			return BehaviorTree::State::Successful;
		}
	}
	return BehaviorTree::State::Failure;
}
