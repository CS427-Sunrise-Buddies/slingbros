#include "windy_grass.hpp"
#include "render.hpp"
#include "animation.hpp"

ECS_ENTT::Entity WindyGrass::createGrass(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity windyGrassEntity = scene->CreateEntity("Windy Grass");

	std::string key = "windygrass";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("tile_blocks_spritesheet.png"), "textured", { 0, 0 });
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache).
	windyGrassEntity.AddComponent<ShadedMeshRef>(resource);

	// Reset the SlingBro colour when created
	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	// Setting up the position.
	Motion& motionComponent = windyGrassEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	windyGrassEntity.AddComponent<WindyGrass>();
	windyGrassEntity.AddComponent<IgnorePhysics>();

	// Set up the animation component
	windyGrassEntity.AddComponent<Animation>(key, glm::vec2(0, 0), 4, 420.0f, false);

	return windyGrassEntity;
}
