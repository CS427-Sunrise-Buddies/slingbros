#include "projectedPath.hpp"
#include "render.hpp"

ECS_ENTT::Entity ProjectedPath::createProjectedPoint(vec3 position, float scale, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource("starPath");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("starPath.png"), "textured");
	}

	ECS_ENTT::Entity projectedPointEntity = scene->CreateEntity("Projected Point");
	projectedPointEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = projectedPointEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 0.5f};

	Motion& motionComponent = projectedPointEntity.AddComponent<Motion>();

	projectedPointEntity.AddComponent<IgnorePhysics>();
	projectedPointEntity.AddComponent<IgnoreSave>();
	ProjectedPath& projectedPathComponent = projectedPointEntity.AddComponent<ProjectedPath>();
	projectedPathComponent.scale = scale;

	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * scale, resource.reference_to_cache->mesh.original_size.y * scale, 1.0f };
	return projectedPointEntity;
}