#include "glass_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity GlassTile::createGlassTile(vec3 position, ECS_ENTT::Scene* scene)
{
	ShadedMesh& meshResource = cache_resource("tile_glass");
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createSprite(meshResource, textures_path("tile_glass.png"), "textured");
	}

	ECS_ENTT::Entity glassTileEntity = scene->CreateEntity("Glass Tile");
	glassTileEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = glassTileEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = glassTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * SPRITE_SCALE, resource.reference_to_cache->mesh.original_size.y * SPRITE_SCALE, 1.0f };

	glassTileEntity.AddComponent<BouncyTile>();
	glassTileEntity.AddComponent<Tile>();
	glassTileEntity.AddComponent<GlassTile>();
	glassTileEntity.AddComponent<IgnorePhysics>();

	return glassTileEntity;
}