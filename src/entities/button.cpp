#include "button.hpp"
#include "render.hpp"


ECS_ENTT::Entity Button::createButton(vec2 position, vec2 scale, const std::string& functionName, ECS_ENTT::Scene* scene)
{
	std::string buttonName = "button_" + functionName;

	ShadedMesh& meshResource = cache_resource(buttonName);
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path(png_file(buttonName)), "textured");
	}

	ECS_ENTT::Entity buttonEntity = scene->CreateEntity(buttonName);
	buttonEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = buttonEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };
	
	Motion& motionComponent = buttonEntity.AddComponent<Motion>();
	motionComponent.position = vec3(position, 0.f);
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * scale.x, resource.reference_to_cache->mesh.original_size.y * scale.y, 1.0f };

	auto& clickableComponent = buttonEntity.AddComponent<ClickableText>();
	clickableComponent.functionName = functionName;

	return buttonEntity;
}
