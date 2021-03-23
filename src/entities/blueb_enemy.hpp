#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

const size_t BLUEB_ENEMY_STEPS_BEFORE_TURN = 250.0;

struct BluebEnemy {

	static ECS_ENTT::Entity createBluebEnemy(vec3 position, ECS_ENTT::Scene* scene);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;

};