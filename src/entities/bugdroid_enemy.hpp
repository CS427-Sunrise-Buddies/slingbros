#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

const size_t BUGDROID_ENEMY_MAX_SHOOT_RANGE = 500.0;

struct BugDroidEnemy {

	static ECS_ENTT::Entity createBugDroidEnemy(vec3 position, ECS_ENTT::Scene* scene);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};