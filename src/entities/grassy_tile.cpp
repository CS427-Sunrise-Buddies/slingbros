#include "grassy_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity GrassyTile::createGrassyTile(vec3 position, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource("tile_ground_grassy");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("tile_ground_grassy.png"), "textured");
	}

	ECS_ENTT::Entity grassyTileEntity = scene->CreateEntity("Grassy Tile");
	grassyTileEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = grassyTileEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = grassyTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * SPRITE_SCALE, resource.reference_to_cache->mesh.original_size.y * SPRITE_SCALE, 1.0f };

	grassyTileEntity.AddComponent<BouncyTile>();
	grassyTileEntity.AddComponent<Tile>();
	grassyTileEntity.AddComponent<GrassyTile>();
	grassyTileEntity.AddComponent<IgnorePhysics>();

	return grassyTileEntity;
}