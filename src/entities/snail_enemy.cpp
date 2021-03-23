#include "snail_enemy.hpp"
#include "render.hpp"
#include "ai/behavioral_tree.hpp"
#include "ai/pathfinding.hpp"

ECS_ENTT::Entity SnailEnemy::createSnailEnemy(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity snailEnemyEntity = scene->CreateEntity("Snail Enemy");

	std::string key = "enemy_snail";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("enemy_snail.png"), "textured");
	}

	snailEnemyEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = snailEnemyEntity.AddComponent<Motion>();
	motionComponent.position = position - glm::vec3(0.0f, 0.0f, 2.0f);
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	AI& aiComponent = snailEnemyEntity.AddComponent<AI>();
	auto* root = new MoveToGoal();
	aiComponent.behavior_tree = root;

	snailEnemyEntity.AddComponent<SnailEnemy>();

	return snailEnemyEntity;
}