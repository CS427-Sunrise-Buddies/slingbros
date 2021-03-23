#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct GlassTile {
	static ECS_ENTT::Entity createGlassTile(vec3 position, ECS_ENTT::Scene* scene);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};