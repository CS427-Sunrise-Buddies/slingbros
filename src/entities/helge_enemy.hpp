#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

const size_t HELGE_ENEMY_MAX_SHOOT_RANGE = 10000.0;
const size_t HELGE_ENEMY_STEPS_BEFORE_TURN = 800;

struct HelgeEnemy
{
	static ECS_ENTT::Entity createHelgeEnemy(vec3 position, ECS_ENTT::Scene* scene);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};