#include "size_down_powerup.hpp"
#include "render.hpp"
#include "powerup.hpp"

ECS_ENTT::Entity SizeDownPowerUp::createSizeDownPowerUp(vec3 position, ECS_ENTT::Scene* scene) {
	ECS_ENTT::Entity sizeDownPowerUpEntity = scene->CreateEntity("Size Down PowerUp");

	std::string key = "powerup_size_down";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("powerup_size_down.png"), "textured");
	}

	sizeDownPowerUpEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = sizeDownPowerUpEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	sizeDownPowerUpEntity.AddComponent<PowerUp>();
	sizeDownPowerUpEntity.AddComponent<SizeDownPowerUp>();

	return sizeDownPowerUpEntity;
}

void SizeDownPowerUp::applyPowerUp(ECS_ENTT::Entity entity) {
	auto& sizeChangeComponent = entity.HasComponent<SizeChanged>() ? entity.GetComponent<SizeChanged>() : entity.AddComponent<SizeChanged>();
	sizeChangeComponent.turnsRemaining = SIZE_DOWN_NUMBER_TURNS;

	auto& motionComponent = entity.GetComponent<Motion>();
	motionComponent.scale = { SIZE_DOWN_SCALE, SIZE_DOWN_SCALE, 1.0f };
}