#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

const size_t MAX_ATTACK_RANGE = 250.0;
const size_t MAX_SHOOT_RANGE = 500.0;
const size_t STEPS_BEFORE_TURN = 250;

struct BasicEnemy
{

	static ECS_ENTT::Entity createBasicEnemy(vec3 position, ECS_ENTT::Scene* scene);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};