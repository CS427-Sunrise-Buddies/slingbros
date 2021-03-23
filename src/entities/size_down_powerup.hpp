#include "Entity.h"
#include "common.hpp"

const float SIZE_DOWN_SCALE = 65.f;
const int SIZE_DOWN_NUMBER_TURNS = 2;

struct SizeDownPowerUp {
	static ECS_ENTT::Entity createSizeDownPowerUp(vec3 position, ECS_ENTT::Scene* scene);

	void applyPowerUp(ECS_ENTT::Entity entity);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};