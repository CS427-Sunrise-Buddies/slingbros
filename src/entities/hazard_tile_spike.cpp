#include "hazard_tile_spike.hpp"
#include "render.hpp"

ECS_ENTT::Entity HazardTileSpike::createHazardTileSpike(vec3 position, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource("hazard_tile_spike");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("hazard_tile_spike.png"), "textured");
	}

	ECS_ENTT::Entity hazardTileSpikeEntity = scene->CreateEntity("Hazard Tile Spike");
	hazardTileSpikeEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = hazardTileSpikeEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	Motion& motionComponent = hazardTileSpikeEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * SPRITE_SCALE, resource.reference_to_cache->mesh.original_size.y * SPRITE_SCALE, 1.0f };

	hazardTileSpikeEntity.AddComponent<Hazard>();
	hazardTileSpikeEntity.AddComponent<BouncyTile>();
	hazardTileSpikeEntity.AddComponent<Tile>();
	hazardTileSpikeEntity.AddComponent<HazardTileSpike>();
	hazardTileSpikeEntity.AddComponent<IgnorePhysics>();

	return hazardTileSpikeEntity;
}