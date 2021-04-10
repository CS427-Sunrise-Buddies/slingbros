#include "goal_tile.hpp"
#include "render.hpp"

ECS_ENTT::Entity GoalTile::createGoalTile(vec3 position, ECS_ENTT::Scene* scene) {
	ECS_ENTT::Entity goalTileEntity = scene->CreateEntity("Goal Tile");

	std::string key = "tile_goal";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("tile_goal.png"), "textured");
	}

	goalTileEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

	Motion& motionComponent = goalTileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	goalTileEntity.AddComponent<Tile>();
	goalTileEntity.AddComponent<GoalTile>();
	goalTileEntity.AddComponent<IgnorePhysics>();

	return goalTileEntity;
}