#include "beehive_enemy.hpp"
#include "render.hpp"

ECS_ENTT::Entity BeeHiveEnemy::createBeeHiveEnemy(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity beehiveEntity = scene->CreateEntity("Bee Hive Enemy");

	std::string key = "enemy_beehive";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("enemy_beehive.png"), "textured");
	}

	beehiveEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	Motion& motionComponent = beehiveEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };
	motionComponent.can_move = false;

	BeeHiveEnemy& beeHiveComponent = beehiveEntity.AddComponent<BeeHiveEnemy>();

	// Create a swarm of bees around the hive
	ParticleSystem* particleSystem = ParticleSystem::GetInstance();
	beeHiveComponent.hiveSwarm = particleSystem->CreateBeeSwarm(position, NUM_BEES_PER_SWARM);
	beeHiveComponent.hiveSwarm->scene = scene;

	return beehiveEntity;
}

bool BeeHiveEnemy::HasBeenHarvestedByPlayer(uint32_t entityID) const
{
	for (uint32_t playerEntityID : harvestedByPlayers)
		if (playerEntityID == entityID)
			return true;
	return false;
}
