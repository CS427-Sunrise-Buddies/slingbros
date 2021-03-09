#include "help_screen.hpp"
#include "render.hpp"


ECS_ENTT::Entity HelpScreen::createHelpScreen(ECS_ENTT::Scene* scene)
{
	ECS_ENTT::Entity helpScreenEntity = scene->CreateEntity("HelpScreen");
	
	ShadedMesh& meshResource = cache_resource("menu_help");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("menu_help.png"), "textured");
	}

	helpScreenEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = helpScreenEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };
	
	
	helpScreenEntity.AddComponent<HelpScreen>();

	Motion& motionComponent = helpScreenEntity.AddComponent<Motion>();
	motionComponent.position = vec3(400.f,300.f,0.f);
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * 1200.f, resource.reference_to_cache->mesh.original_size.y * 800.f, 1.0f };
	
	
	return helpScreenEntity;
}

