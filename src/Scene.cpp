#include "Scene.h"

#include <utility>
#include "Entity.h"
#include "common.hpp"

// TODO: still need a file which defines all the Component structs we will be using
//#include "Components.h"

namespace ECS_ENTT {

	Scene::Scene(std::string name, glm::vec2 size) :
		m_Name(std::move(name)),
		m_Size(vec2(size.x * SPRITE_SCALE, size.y * SPRITE_SCALE)),
		m_Map(LevelMap(size.y,LevelRow(size.x)))
		{};

	// Create an entity and associate it to this Scene
	Entity Scene::CreateEntity(const std::string& name)
	{
		// Create the entity from a handle
		entt::entity entityHandle = m_Registry.create();
		ECS_ENTT::Entity entity = Entity(entityHandle, this);

		// TODO: might want to add a TransformComponent to all entities
		//entity.AddComponent<TransformComponent>();

		// TODO: might want to add a TagComponent to all entities
		//auto& tagComponent = entity.AddComponent<TagComponent>(name);
		//tagComponent.Tag = name.empty() ? "Unnamed Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdate(float deltaTime)
	{
		// TODO: Here we can find the camera entity and then use it to render this Scene
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		// Never supposed to not have a specialization for a component type
		// (see below for examples of specialized overloads)
		assert(false);
	}

	// Examples on how we can later use OnComponentAdded functions for each type of component

	//template<>
	//void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& cameraComponent)
	//{
	//	cameraComponent.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	//}

	//template<>
	//void Scene::OnComponentAdded<ExampleComponent>(Entity entity, ExampleComponent& exampleComponent)
	//{
	//}

}