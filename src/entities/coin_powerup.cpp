#include "coin_powerup.hpp"
#include "render.hpp"
#include "powerup.hpp"

ECS_ENTT::Entity CoinPowerUp::createCoinPowerUp(vec3 position, ECS_ENTT::Scene* scene) {
	ECS_ENTT::Entity coinPowerUpEntity = scene->CreateEntity("Coin PowerUp");

	std::string key = "powerup_coin";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("powerup_coin.png"), "textured");
	}

	coinPowerUpEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = coinPowerUpEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE * 0.8f, resource.mesh.original_size.y * SPRITE_SCALE * 0.8f, 1.0f };

	coinPowerUpEntity.AddComponent<PowerUp>();
	coinPowerUpEntity.AddComponent<CoinPowerUp>();

	return coinPowerUpEntity;
}

void CoinPowerUp::applyPowerUp(ECS_ENTT::Entity entity) {
	Turn& turn = entity.GetComponent<Turn>();
	turn.addPoints(POINTS_GAINED_COIN_VALUE);
}
