#include "Entity.h"
#include "common.hpp"

struct SpeedPowerUp {
	static ECS_ENTT::Entity createSpeedPowerUp(vec3 position, ECS_ENTT::Scene* scene);

	void applyPowerUp(ECS_ENTT::Entity entity);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};