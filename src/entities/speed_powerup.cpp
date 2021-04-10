#include "speed_powerup.hpp"
#include "render.hpp"
#include "powerup.hpp"

ECS_ENTT::Entity SpeedPowerUp::createSpeedPowerUp(vec3 position, ECS_ENTT::Scene* scene) {
	ECS_ENTT::Entity speedPowerUpEntity = scene->CreateEntity("Speed PowerUp");

	std::string key = "powerup_speed";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("powerup_speed.png"), "textured");
	}

	speedPowerUpEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	Motion& motionComponent = speedPowerUpEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	speedPowerUpEntity.AddComponent<PowerUp>();
	speedPowerUpEntity.AddComponent<SpeedPowerUp>();

	return speedPowerUpEntity;
}

void SpeedPowerUp::applyPowerUp(ECS_ENTT::Entity entity) {
	Motion& motion = entity.GetComponent<Motion>();
	motion.velocity *= 2;
}