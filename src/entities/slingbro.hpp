#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"

struct OrangeBro {
	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};

struct PinkBro {
	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};

struct SlingBro {
	static ECS_ENTT::Entity createOrangeSlingBro(vec3 position, ECS_ENTT::Scene* scene);

	static ECS_ENTT::Entity createPinkSlingBro(vec3 position, ECS_ENTT::Scene* scene);

	static ECS_ENTT::Entity createOrangeSlingBroProfile(vec3 position, ECS_ENTT::Scene* scene);

	static ECS_ENTT::Entity createPinkSlingBroProfile(vec3 position, ECS_ENTT::Scene* scene);

	static ECS_ENTT::Entity createSlingBro(vec3 position, ECS_ENTT::Scene* scene, BroType type, bool isProfile);

	// Bug fix for now, just adding something here so that this component isn't empty since apparently EnTT doesn't like empty components
	uint32_t placeholder = 0;
};

struct Deformation {
	Deformation(float scaleX, float scaleY, float angleRadians, float deformationTimeMs)
		: scaleX(scaleX), scaleY(scaleY), angleRadians(angleRadians), timeRemaining(deformationTimeMs)
	{}
	float timeRemaining;
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float angleRadians = 0.0f;
};