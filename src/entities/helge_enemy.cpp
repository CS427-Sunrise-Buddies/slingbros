#include "helge_enemy.hpp"
#include "render.hpp"
#include "animation.hpp"

#include <ai/behavioral_tree.hpp>
#include <ai/attack.hpp>
#include <ai/movement.hpp>

ECS_ENTT::Entity HelgeEnemy::createHelgeEnemy(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity helgeEnemyEntity = scene->CreateEntity("Helge Enemy");

	std::string key = "enemy_helge";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("enemy_characters_spritesheet.png"), "textured", { 0, 2 });
	}

	helgeEnemyEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = helgeEnemyEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { AI_SPEED * 2, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE * 2.5f, resource.mesh.original_size.y * SPRITE_SCALE * 2.5f, 1.0f };
	motionComponent.can_move = false;

	AI& aiComponent = helgeEnemyEntity.AddComponent<AI>();
	auto* shootNode = new HelgeShootInRange(HELGE_ENEMY_MAX_SHOOT_RANGE);
	auto* patrolNode = new Patrol(HELGE_ENEMY_STEPS_BEFORE_TURN);
	auto* root = new BehaviorTree::Selector(std::vector<BehaviorTree::Node*>({ shootNode, patrolNode }));
	aiComponent.behavior_tree = root;

	helgeEnemyEntity.AddComponent<HelgeEnemy>();

	helgeEnemyEntity.AddComponent<Animation>(key, glm::vec2(0, 2), 7, 200.0f, true);

	return helgeEnemyEntity;
}