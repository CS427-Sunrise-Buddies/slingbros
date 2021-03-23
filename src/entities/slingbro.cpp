#include "slingbro.hpp"
#include "render.hpp"
#include "animation.hpp"

ECS_ENTT::Entity OrangeBro::createOrangeSlingBro(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity slingBroEntity = scene->CreateEntity("SlingBro");

	std::string key = "slingbro";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0)
		RenderSystem::createSprite(resource, textures_path("bro_characters_spritesheet.png"), "textured", { 0, 0 });

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache).
	slingBroEntity.AddComponent<ShadedMeshRef>(resource);

	// Reset the SlingBro colour when created
	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	// Setting up SlingBro position.
	Motion& motionComponent = slingBroEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	slingBroEntity.AddComponent<OrangeBro>();
	slingBroEntity.AddComponent<SlingBro>();
	SlingMotion& slingComponent = slingBroEntity.AddComponent<SlingMotion>();
	slingComponent.isClicked = false;
	slingComponent.direction = { 0.0f, 0.0f };
	slingComponent.magnitude = { 10.f, 20.f };

	// Add countdown for ending turn
	slingBroEntity.AddComponent<Turn>();

	// Set up the animation component
	slingBroEntity.AddComponent<Animation>(key, glm::vec2(0, 0), 6, 200.0f, true);
	slingBroEntity.AddComponent<Gravity>();

	return slingBroEntity;
}

ECS_ENTT::Entity PinkBro::createPinkSlingBro(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity slingBroEntity = scene->CreateEntity("SlingBro2");

	std::string key = "slingbro2";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0)
		RenderSystem::createSprite(resource, textures_path("bro_characters_spritesheet.png"), "textured", { 0, 2 });

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache).
	slingBroEntity.AddComponent<ShadedMeshRef>(resource);

	// Reset the SlingBro colour when created
	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	// Setting up SlingBro position.
	Motion& motionComponent = slingBroEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	slingBroEntity.AddComponent<PinkBro>();
	slingBroEntity.AddComponent<SlingBro>();
	SlingMotion& slingComponent = slingBroEntity.AddComponent<SlingMotion>();
	slingComponent.isClicked = false;
	slingComponent.direction = { 0.0f, 0.0f };
	slingComponent.magnitude = { 10.f, 20.f };

	// Add countdown for ending turn
	slingBroEntity.AddComponent<Turn>();

	// Set up the animation component
	slingBroEntity.AddComponent<Animation>(key, glm::vec2(0, 2), 6, 200.0f, true);
	slingBroEntity.AddComponent<Gravity>();

	return slingBroEntity;
}