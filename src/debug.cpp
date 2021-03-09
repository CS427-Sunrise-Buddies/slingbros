// Header
#include "debug.hpp"
//#include "tiny_ecs.hpp"
#include "render.hpp"

#include <cmath>
#include <iostream>

#include "render_components.hpp"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 2
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
namespace DebugSystem
{
	void createLine(vec3 position, vec3 scale)
	{
		auto entity = WorldSystem::ActiveScene->CreateEntity("Debug");

		std::string key = "thick_line";
		ShadedMesh& resource = cache_resource(key);

		if (resource.effect.program.resource == 0) {

			// Create a procedural circle.
			constexpr float z = -0.1f;
			vec3 red = { 0.8,0.1,0.1 };

			//Corner points.
			ColoredVertex v;
			v.position = { -0.5,-0.5,z };
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
		entity.AddComponent<ShadedMeshRef>(resource);

		// Create motion
		auto& motion = entity.AddComponent<Motion>();
		motion.angle = 0.f;
		motion.velocity = { 0, 0, 0 };
		motion.position = position;
		motion.scale = scale;

		entity.AddComponent<DebugComponent>();
	}

	void createBox(vec3 position, vec2 bounding_box, vec3 scale) {
		// Create top line.
		createLine({ position.x, position.y - (bounding_box.y / 2.0), position.z }, { scale.x, scale.y * 0.05, scale.z });

		// Create bottom line.
		createLine({ position.x, position.y + (bounding_box.y / 2.0), position.z }, { scale.x, scale.y * 0.05, scale.z });

		// Create left line.
		createLine({ position.x - (bounding_box.x / 2.0), position.y, position.z }, { scale.x * 0.05, scale.y, scale.z });

		// Create right line.
		createLine({ position.x + (bounding_box.x / 2.0), position.y, position.z }, { scale.x * 0.05, scale.y, scale.z });
	}

	void createCircle(vec3 position, vec3 scale) {
		auto entity = WorldSystem::ActiveScene->CreateEntity("Debug Circle");
		std::string key = "debug_circle";
		ShadedMesh& resource = cache_resource(key);

		if (resource.effect.program.resource == 0) {
			RenderSystem::createSprite(resource, textures_path("debug_circle.png"), "textured");
		}
		entity.AddComponent<ShadedMeshRef>(resource);

		auto& motion = entity.AddComponent<Motion>();
		motion.angle = 0.f;
		motion.velocity = { 0, 0, 0 };
		motion.position = position;
		motion.scale = scale;

		entity.AddComponent<DebugComponent>();
	}

	void clearDebugComponents()
	{
		auto debugEntitiesView = WorldSystem::ActiveScene->m_Registry.view<DebugComponent>();

		for (auto debugEntity : debugEntitiesView) {
			WorldSystem::ActiveScene->m_Registry.destroy(debugEntity);
		}
	}

	bool in_debug_mode = false;
	bool in_freeze_mode = false;
	float freeze_delay_ms = 1000;

}
