#include "spike_hazard.hpp"
#include "render.hpp"

ECS_ENTT::Entity HazardSpike::createSpikeHazard(vec3 position, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource("hazard_spike");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("hazard_spike.png"), "textured");
	}

	ECS_ENTT::Entity hazardSpikeEntity = scene->CreateEntity("Spike Hazard");
	hazardSpikeEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = hazardSpikeEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = hazardSpikeEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * SPRITE_SCALE, resource.reference_to_cache->mesh.original_size.y * SPRITE_SCALE, 1.0f };

	hazardSpikeEntity.AddComponent<HazardSpike>();
	hazardSpikeEntity.AddComponent<IgnorePhysics>();

	return hazardSpikeEntity;
}