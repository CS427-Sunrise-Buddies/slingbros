#include "Entity.h"
#include "common.hpp"

const float SIZE_UP_SCALE = 150.f;
const int SIZE_UP_NUMBER_TURNS = 2;

struct SizeUpPowerUp {
	static ECS_ENTT::Entity createSizeUpPowerUp(vec3 position, ECS_ENTT::Scene* scene);

	void applyPowerUp(ECS_ENTT::Entity entity);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};