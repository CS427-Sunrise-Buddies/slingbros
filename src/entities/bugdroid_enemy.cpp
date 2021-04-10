#include "bugdroid_enemy.hpp"
#include "render.hpp"

#include <ai/behavioral_tree.hpp>
#include <ai/attack.hpp>

ECS_ENTT::Entity BugDroidEnemy::createBugDroidEnemy(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity bugDroidEnemyEntity = scene->CreateEntity("BugDroid Enemy");

	std::string key = "enemy_bugdroid";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("enemy_bugdroid.png"), "textured");
	}

	bugDroidEnemyEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	Motion& motionComponent = bugDroidEnemyEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	// InitiateAI components
	AI& aiComponent = bugDroidEnemyEntity.AddComponent<AI>();
	auto* shootNode = new ShootInRange(BUGDROID_ENEMY_MAX_SHOOT_RANGE);
	auto* root = new BehaviorTree::Selector(std::vector<BehaviorTree::Node*>({ shootNode }));
	aiComponent.behavior_tree = root;

	bugDroidEnemyEntity.AddComponent<BugDroidEnemy>();
	bugDroidEnemyEntity.AddComponent<CollidableEnemy>();

	return bugDroidEnemyEntity;
}