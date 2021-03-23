#include "size_up_powerup.hpp"
#include "render.hpp"
#include "powerup.hpp"

ECS_ENTT::Entity SizeUpPowerUp::createSizeUpPowerUp(vec3 position, ECS_ENTT::Scene* scene) {
	ECS_ENTT::Entity sizeUpPowerUpEntity = scene->CreateEntity("Size Up PowerUp");

	std::string key = "powerup_size_up";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("powerup_size_up.png"), "textured");
	}

	sizeUpPowerUpEntity.AddComponent<ShadedMeshRef>(resource);
	
	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = sizeUpPowerUpEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	sizeUpPowerUpEntity.AddComponent<PowerUp>();
	sizeUpPowerUpEntity.AddComponent<SizeUpPowerUp>();

	return sizeUpPowerUpEntity;
}

void SizeUpPowerUp::applyPowerUp(ECS_ENTT::Entity entity) {

	auto& sizeChangeComponent = entity.HasComponent<SizeChanged>() ? entity.GetComponent<SizeChanged>() : entity.AddComponent<SizeChanged>();
	sizeChangeComponent.turnsRemaining = SIZE_UP_NUMBER_TURNS;

	auto& motionComponent = entity.GetComponent<Motion>();
	motionComponent.scale = { SIZE_UP_SCALE, SIZE_UP_SCALE, 1.0f };
}