#include "ground_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity GroundTile::createGroundTile(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity groundTileEntity = scene->CreateEntity("Ground Tile");

	std::string key = "ground_tile";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("ground_tile.png"), "textured");
	}

	groundTileEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = groundTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * 100.f, resource.mesh.original_size.y * 100.f, 1.0f };

	groundTileEntity.AddComponent<GroundTile>();

	return groundTileEntity;
}