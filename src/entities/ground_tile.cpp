#include "ground_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity GroundTile::createGroundTile(vec3 position, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource("tile_ground");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("tile_ground.png"), "textured");
	}

	ECS_ENTT::Entity groundTileEntity = scene->CreateEntity("Ground Tile");
	groundTileEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = groundTileEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = groundTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * SPRITE_SCALE, resource.reference_to_cache->mesh.original_size.y * SPRITE_SCALE, 1.0f };

	groundTileEntity.AddComponent<BouncyTile>();
	groundTileEntity.AddComponent<Tile>();
	groundTileEntity.AddComponent<GroundTile>();

	return groundTileEntity;
}