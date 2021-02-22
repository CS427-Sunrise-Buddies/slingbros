#include "start_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity StartTile::createStartTile(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity startTileEntity = scene->CreateEntity("Start Tile");

	std::string key = "start_tile";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("start_tile.png"), "textured");
	}

	startTileEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = startTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * 100.f, resource.mesh.original_size.y * 100.f, 1.0f };

	startTileEntity.AddComponent<StartTile>();

	return startTileEntity;
}