// Header
#include "projectile.hpp"
#include "render.hpp"

ECS_ENTT::Entity Projectile::createProjectile(vec3 position, ECS_ENTT::Scene *scene)
{
	ECS_ENTT::Entity projectileEntity = scene->CreateEntity("Projectile");

	std::string key = "projectile";
	ShadedMesh& resource = cache_resource(key);
	if (resource.mesh.vertices.empty())
	{
		resource.mesh.loadFromOBJFile(mesh_path("circle.obj"));
		RenderSystem::createColoredMesh(resource, "projectile");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	projectileEntity.AddComponent<ShadedMeshRef>(resource);

	// Reset the projectile colour when created
	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	// Setting initial motion values
	auto& motionComponent = projectileEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * 20.f, resource.mesh.original_size.y * 20.f, 1.0f };

	// Create an (empty) projectile component to be able to refer to all projectiles
	projectileEntity.AddComponent<Projectile>(); // TODO check if we can't add empty components, seems to cause a bug 

	return projectileEntity;
}
