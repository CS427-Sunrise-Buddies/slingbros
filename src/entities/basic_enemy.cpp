#include "basic_enemy.hpp"
#include "render.hpp"
#include "animation.hpp"

#include <ai/behavioral_tree.hpp>
#include <ai/attack.hpp>
#include <ai/movement.hpp>

ECS_ENTT::Entity BasicEnemy::createBasicEnemy(vec3 position, ECS_ENTT::Scene* scene)
{

	ECS_ENTT::Entity basicEnemyEntity = scene->CreateEntity("Basic Enemy");

	std::string key = "enemy_basic";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("enemy_basic.png"), "textured");
	}

	basicEnemyEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec3{1.0f, 1.0f, 1.0f};

	Motion& motionComponent = basicEnemyEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = {50.0f, 0.0f, 0.0f};
	motionComponent.scale = {resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f};

	// Initiate AI component + Behavior Tree for Basic Enemy
	AI& aiComponent = basicEnemyEntity.AddComponent<AI>();
	auto* shootNode = new ShootInRange(MAX_SHOOT_RANGE);
	auto* patrolNode = new Patrol(STEPS_BEFORE_TURN);
	auto* root = new BehaviorTree::Selector(std::vector<BehaviorTree::Node*>({shootNode, patrolNode}));
	aiComponent.behavior_tree = root;

	basicEnemyEntity.AddComponent<BasicEnemy>();

	// Set up the animation component
	basicEnemyEntity.AddComponent<Animation>(key, textures_path("test_spritesheet.png"), glm::vec2(0, 2), 7, 200.0f);

	return basicEnemyEntity;
}