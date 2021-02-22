#include "basic_enemy.hpp"
#include "render.hpp"

#include <ai/behavioral_tree.hpp>
#include <ai/attack.hpp>
#include <ai/movement.hpp>

ECS_ENTT::Entity BasicEnemy::createBasicEnemy(vec3 position, ECS_ENTT::Scene* scene)
{

	ECS_ENTT::Entity basicEnemyEntity = scene->CreateEntity("Basic Enemy");

	std::string key = "basic_enemy";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("basic_enemy.png"), "textured");
	}

	basicEnemyEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec3{1.0f, 1.0f, 1.0f};

	Motion& motionComponent = basicEnemyEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = {50.0f, 0.0f, 0.0f};
	motionComponent.scale = {resource.mesh.original_size.x * 100.f, resource.mesh.original_size.y * 100.f, 1.0f};

	// Initiate AI component + Behavior Tree for Basic Enemy
	AI& aiComponent = basicEnemyEntity.AddComponent<AI>();

	auto* attackNode = new AttackInRange(MAX_ATTACK_RANGE);
	auto* shootNode = new ShootInRange(MAX_SHOOT_RANGE);

	auto* patrolNode = new Patrol(STEPS_BEFORE_TURN);

	auto* root = new BehaviorTree::Selector(std::vector<BehaviorTree::Node*>({attackNode, shootNode, patrolNode}));

	aiComponent.behavior_tree = root;
	aiComponent.behavior_tree->init(basicEnemyEntity);

	basicEnemyEntity.AddComponent<BasicEnemy>();

	return basicEnemyEntity;
}