#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct ProjectedPath {

	static ECS_ENTT::Entity createProjectedPoint(vec3 position, float scale, ECS_ENTT::Scene* scene);

	float scale;
};