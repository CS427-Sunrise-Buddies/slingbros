#include "Entity.h"
#include "common.hpp"

const float MASS_UP_VALUE = 10.0f;
const int MASS_UP_NUMBER_TURNS = 2;

struct MassUpPowerUp {
	static ECS_ENTT::Entity createMassUpPowerUp(vec3 position, ECS_ENTT::Scene* scene);

	void applyPowerUp(ECS_ENTT::Entity entity);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};