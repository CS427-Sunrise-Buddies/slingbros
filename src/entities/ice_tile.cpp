#include "ice_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity IceTile::createIceTile(vec3 position, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource("tile_ice");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("tile_ice.png"), "textured");
	}

	ECS_ENTT::Entity snowyTileEntity = scene->CreateEntity("Ice Tile");
	snowyTileEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = snowyTileEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = snowyTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * SPRITE_SCALE, resource.reference_to_cache->mesh.original_size.y * SPRITE_SCALE, 1.0f };

	snowyTileEntity.AddComponent<BouncyTile>();
	snowyTileEntity.AddComponent<Tile>();
	snowyTileEntity.AddComponent<IceTile>();
	snowyTileEntity.AddComponent<IgnorePhysics>();

	return snowyTileEntity;
}