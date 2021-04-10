#include "helge_projectile.hpp"
#include "projectile.hpp"
#include "render.hpp"

ECS_ENTT::Entity HelgeProjectile::createHelgeProjectile(vec3 position, ECS_ENTT::Scene* scene) {
	ECS_ENTT::Entity helgeProjectileEntity = scene->CreateEntity("Helge Projectile");

	std::string key = "projectile_helge";
	ShadedMesh& resource = cache_resource(key);

	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path("projectile_f_minus.png"), "textured");
	}

	helgeProjectileEntity.AddComponent<ShadedMeshRef>(resource);

	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

	auto& motionComponent = helgeProjectileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * 50.f, resource.mesh.original_size.y * 50.f, 1.0f };

	helgeProjectileEntity.AddComponent<HelgeProjectile>();

	return helgeProjectileEntity;
}