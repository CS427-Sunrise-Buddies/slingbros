#include "screen.hpp"
#include "render.hpp"


ECS_ENTT::Entity GameScreen::createScreen(const std::string& texture_name, vec2 pos, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource(texture_name);
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path(png_file(texture_name)), "textured");
	}

	ECS_ENTT::Entity screenEntity = scene->CreateEntity(texture_name);
	screenEntity.AddComponent<ShadedMeshRef>(meshResource);
	auto& resource = screenEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	auto& motionComponent = screenEntity.AddComponent<Motion>();
	motionComponent.position = vec3(pos.x, pos.y, 0.f);
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * WINDOW_SIZE_IN_PX.x, resource.reference_to_cache->mesh.original_size.y * WINDOW_SIZE_IN_PX.y, 1.0f };

	screenEntity.AddComponent<IgnorePhysics>();
	
	return screenEntity;
}

