#include "blueb_enemy.hpp"
#include "render.hpp"

#include <ai/behavioral_tree.hpp>
#include <ai/movement.hpp>

ECS_ENTT::Entity BluebEnemy::createBluebEnemy(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity bluebEnemyEntity = scene->CreateEntity("Blueb Enemy");

	std::string key = "enemy_blueb";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("enemy_blueb.png"), "textured");
	}

	bluebEnemyEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = bluebEnemyEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 50.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };
	motionComponent.can_move = false;

	// InitiateAI components
	AI& aiComponent = bluebEnemyEntity.AddComponent<AI>();
	auto* patrolNode = new Patrol(BLUEB_ENEMY_STEPS_BEFORE_TURN);
	auto* root = new BehaviorTree::Selector(std::vector<BehaviorTree::Node*>({ patrolNode }));
	aiComponent.behavior_tree = root;

	bluebEnemyEntity.AddComponent<BluebEnemy>();

	return bluebEnemyEntity;
}