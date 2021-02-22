#include "goal_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity GoalTile::createGoalTile(vec3 position, ECS_ENTT::Scene* scene) {

	ECS_ENTT::Entity goalTileEntity = scene->CreateEntity("Goal Tile");

	std::string key = "goal_tile";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("goal_tile.png"), "textured");
	}

	goalTileEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = goalTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * 100.f, resource.mesh.original_size.y * 100.f, 1.0f };

	goalTileEntity.AddComponent<GoalTile>();

	return goalTileEntity;
}