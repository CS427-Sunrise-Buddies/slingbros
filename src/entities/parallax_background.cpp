#include "parallax_background.hpp"
#include "render.hpp"

const glm::vec3 BACKGROUND_POSITION = glm::vec3(500.0f, 0.0f, -1000.0f);

uint32_t ParallaxBackground::backgroundID = 0;

ECS_ENTT::Entity ParallaxBackground::createBackground(ECS_ENTT::Scene* scene, std::string backgroundFilename) {

	if (backgroundFilename == "")
		assert(false); 

	ECS_ENTT::Entity backgroundEntity = scene->CreateEntity("Background");

	std::string key = "background_";
	key += std::to_string(backgroundID++);
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) 
		RenderSystem::createBackgroundSprite(resource, textures_path(backgroundFilename), "textured");

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache).
	backgroundEntity.AddComponent<ShadedMeshRef>(resource);

	// Reset the SlingBro colour when created
	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	// Setting up the position.
	Motion& motionComponent = backgroundEntity.AddComponent<Motion>();
	motionComponent.position = BACKGROUND_POSITION;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE * 500.0f, resource.mesh.original_size.y * SPRITE_SCALE * 500.0f, 1.0f };

	backgroundEntity.AddComponent<ParallaxBackground>();
	backgroundEntity.AddComponent<IgnorePhysics>();
	backgroundEntity.AddComponent<IgnoreSave>();

	scene->m_BackgroundFilename = backgroundFilename;

	return backgroundEntity;
}
