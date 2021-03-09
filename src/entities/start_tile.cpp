#include "start_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity StartTile::createStartTile(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity startTileEntity = scene->CreateEntity("Start Tile");

	std::string key = "tile_start";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("tile_start.png"), "textured");
	}

	startTileEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = startTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	startTileEntity.AddComponent<Tile>();
	startTileEntity.AddComponent<StartTile>();

	return startTileEntity;
}