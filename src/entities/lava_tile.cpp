#include "lava_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity LavaTile::createLavaTile(vec3 position, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource("tile_lava");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("tile_lava.png"), "textured");
	}

	ECS_ENTT::Entity lavaTileEntity = scene->CreateEntity("Lava Tile");
	lavaTileEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = lavaTileEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = lavaTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * SPRITE_SCALE, resource.reference_to_cache->mesh.original_size.y * SPRITE_SCALE, 1.0f };

	lavaTileEntity.AddComponent<BouncyTile>();
	lavaTileEntity.AddComponent<Tile>();
	lavaTileEntity.AddComponent<LavaTile>();

	return lavaTileEntity;
}