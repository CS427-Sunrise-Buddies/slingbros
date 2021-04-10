#include "mass_up_powerup.hpp"
#include "render.hpp"
#include "powerup.hpp"

ECS_ENTT::Entity MassUpPowerUp::createMassUpPowerUp(vec3 position, ECS_ENTT::Scene* scene) {
	ECS_ENTT::Entity massUpPowerUpEntity = scene->CreateEntity("Mass Up PowerUp");

	std::string key = "powerup_mass_up";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("powerup_mass_up.png"), "textured");
	}

	massUpPowerUpEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = massUpPowerUpEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	massUpPowerUpEntity.AddComponent<PowerUp>();
	massUpPowerUpEntity.AddComponent<MassUpPowerUp>();

	return massUpPowerUpEntity;
}

void MassUpPowerUp::applyPowerUp(ECS_ENTT::Entity entity) {

	auto& massChangedComponent = entity.HasComponent<MassChanged>() ? entity.GetComponent<MassChanged>() : entity.AddComponent<MassChanged>();
	massChangedComponent.turnsRemaining = MASS_UP_NUMBER_TURNS;

	auto& massComponent = entity.GetComponent<Mass>();
	massComponent.value = MASS_UP_VALUE;
}