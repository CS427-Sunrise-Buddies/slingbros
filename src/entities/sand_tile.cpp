#include "sand_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity SandTile::createSandTile(vec3 position, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource("tile_sand");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("tile_sand.png"), "textured");
	}

	ECS_ENTT::Entity sandTileEntity = scene->CreateEntity("Sand Tile");
	sandTileEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = sandTileEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = sandTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * SPRITE_SCALE, resource.reference_to_cache->mesh.original_size.y * SPRITE_SCALE, 1.0f };

	sandTileEntity.AddComponent<BouncyTile>();
	sandTileEntity.AddComponent<Tile>();
	sandTileEntity.AddComponent<SandTile>();
	sandTileEntity.AddComponent<IgnorePhysics>();

	return sandTileEntity;
}