// Header
#include "salmon.hpp"
#include "render.hpp"

ECS_ENTT::Entity Salmon::createSalmon(vec2 position, ECS_ENTT::Scene* scene)
{
	ECS_ENTT::Entity salmonEntity = scene->CreateEntity("Player Salmon");

	std::string key = "salmon";
	ShadedMesh& resource = cache_resource(key);
	if (resource.mesh.vertices.size() == 0)
	{
		resource.mesh.loadFromOBJFile(mesh_path("salmon.obj"));
		RenderSystem::createColoredMesh(resource, "salmon");
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	salmonEntity.AddComponent<ShadedMeshRef>(resource);
	// Using the above with EnTT, instead of the tiny_ecs way below for reference:
	//ECS::registry<ShadedMeshRef>.emplace(salmonEntity, resource);

	// Reset the salmon colour when created
	resource.texture.color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	// Setting initial motion values
	Motion& motionComponent = salmonEntity.AddComponent<Motion>();
	// Using the above with EnTT, instead of the tiny_ecs way below for reference:
	//Motion& motionComponent = ECS::registry<Motion>.emplace(salmonEntity);
	motionComponent.position = position;
	motionComponent.angle = 0.f;
	motionComponent.velocity = { 0.f, 0.f };
	motionComponent.scale = resource.mesh.original_size * 150.f;
	motionComponent.scale.x *= -1; // point front to the right

	// Create an (empty) Salmon component to be able to refer to all Salmons
	salmonEntity.AddComponent<Salmon>(); // TODO check if we can't add empty components, seems to cause a bug 
	// Using the above with EnTT, instead of the tiny_ecs way below for reference:
	//ECS::registry<Salmon>.emplace(salmonEntity);

	return salmonEntity;
}
