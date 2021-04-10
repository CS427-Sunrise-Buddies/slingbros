#include "bird_enemy.hpp"
#include "render.hpp"

#include <ai/behavioral_tree.hpp>
#include <ai/movement.hpp>

ECS_ENTT::Entity BirdEnemy::createBirdEnemy(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity birdEnemyEntity = scene->CreateEntity("Bird Enemy");

	std::string key = "enemy_bird";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("enemy_bird.png"), "textured");
	}

	birdEnemyEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	Motion& motionComponent = birdEnemyEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 80.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };
	motionComponent.can_move = false;

	// InitiateAI components
	AI& aiComponent = birdEnemyEntity.AddComponent<AI>();
	auto* skyPatrolNode = new SkyPatrol(BIRD_ENEMY_STEPS_BEFORE_TURN);
	auto* root = new BehaviorTree::Selector(std::vector<BehaviorTree::Node*>({ skyPatrolNode }));
	aiComponent.behavior_tree = root;

	birdEnemyEntity.AddComponent<BirdEnemy>();
	birdEnemyEntity.AddComponent<CollidableEnemy>();

	return birdEnemyEntity;
}