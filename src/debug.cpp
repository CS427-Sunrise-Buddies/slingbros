// Header
#include "debug.hpp"
//#include "tiny_ecs.hpp"
#include "render.hpp"

#include <cmath>
#include <iostream>

#include "render_components.hpp"
#include "world.hpp"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 2
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
namespace DebugSystem 
{
	void createLine(vec3 position, vec3 scale, ECS_ENTT::Scene *scene)
	{
		// TODO using EnTT
		ECS_ENTT::Entity debugEntity = scene->CreateEntity("Debug");

		std::string key = "thick_line";
		ShadedMesh& resource = cache_resource(key);
		if (resource.effect.program.resource == 0) {
			// create a procedural circle
			constexpr float z = -0.1f;
			vec3 red = { 0.8,0.1,0.1 };

			// Corner points
			ColoredVertex v;
			v.position = {-0.5,-0.5,z};
			v.color = red;
			resource.mesh.vertices.push_back(v);
			v.position = { -0.5,0.5,z };
			v.color = red;
			resource.mesh.vertices.push_back(v);
			v.position = { 0.5,0.5,z };
			v.color = red;
			resource.mesh.vertices.push_back(v);
			v.position = { 0.5,-0.5,z };
			v.color = red;
			resource.mesh.vertices.push_back(v);

			// Two triangles
			resource.mesh.vertex_indices.push_back(0);
			resource.mesh.vertex_indices.push_back(1);
			resource.mesh.vertex_indices.push_back(3);
			resource.mesh.vertex_indices.push_back(1);
			resource.mesh.vertex_indices.push_back(2);
			resource.mesh.vertex_indices.push_back(3);

			RenderSystem::createColoredMesh(resource, "colored_mesh");
		}

		// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
		debugEntity.AddComponent<ShadedMeshRef>(resource);

		// Create motionComponent
		auto& motionComponent = debugEntity.AddComponent<Motion>();
		motionComponent.angle = 0.f;
		motionComponent.velocity = { 0.f, 0.f, 0.f };
		motionComponent.position = position;
		motionComponent.scale = scale;

		debugEntity.AddComponent<DebugComponent>();
	}

	void clearDebugComponents() 
	{
		// TODO using EnTT
		// Clear old debugging visualizations

//		auto view = WorldSystem::GameScene->m_Registry.view<DebugComponent>();
//		WorldSystem::GameScene->m_Registry.destroy(view.begin(), view.end());
//		while (ECS::registry<DebugComponent>.entities.size() > 0) {
//			ECS::ContainerInterface::remove_all_components_of(ECS::registry<DebugComponent>.entities.back());
//        }
	}

	bool in_debug_mode = false;
	bool is_gravity_on = true;
}
