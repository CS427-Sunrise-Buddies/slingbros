#include "slingbro.hpp"
#include "render.hpp"

ECS_ENTT::Entity SlingBro::createSlingBro(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity slingBroEntity = scene->CreateEntity("SlingBro");

	std::string key = "slingbro";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("test_bro.png"), "textured");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache).
	slingBroEntity.AddComponent<ShadedMeshRef>(resource);

	// Reset the SlingBro colour when created
	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	// Setting up SlingBro position.
	Motion& motionComponent = slingBroEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * 100.f, resource.mesh.original_size.y * 100.f, 1.0f };

	slingBroEntity.AddComponent<SlingBro>();
	SlingMotion& slingComponent = slingBroEntity.AddComponent<SlingMotion>();
	slingComponent.isClicked = false;
	slingComponent.direction = { 0.0f, 0.0f };
	slingComponent.magnitude = 0.f;

	return slingBroEntity;
}
