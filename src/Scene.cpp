#include "Scene.h"

#include <utility>
#include "Entity.h"
#include "common.hpp"

// TODO: still need a file which defines all the Component structs we will be using
//#include "Components.h"

namespace ECS_ENTT {

	Scene::Scene(std::string name, glm::vec2 size) :
		m_Name(std::move(name)),
		m_BackgroundFilename(""),
		m_Size(vec2(size.x * SPRITE_SCALE, size.y * SPRITE_SCALE)),
		m_Camera(new Camera()),
		m_Map(LevelMap(size.y,LevelRow(size.x)))
		{};

	Scene::Scene(std::string name, glm::vec2 size, Camera* camera) :
		m_Name(std::move(name)),
		m_BackgroundFilename(""),
		m_Size(vec2(size.x * SPRITE_SCALE, size.y * SPRITE_SCALE)),
		m_Camera(camera),
		m_Map(LevelMap(size.y,LevelRow(size.x)))
		{};

	// Create an entity and associate it to this Scene
	Entity Scene::CreateEntity(const std::string& name)
	{
		// Create the entity from a handle
		entt::entity entityHandle = m_Registry.create();
		ECS_ENTT::Entity entity = Entity(entityHandle, this);

		// TODO: might want to add a TagComponent to all entities
		//auto& tagComponent = entity.AddComponent<TagComponent>(name);
		//tagComponent.Tag = name.empty() ? "Unnamed Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	unsigned int Scene::GetPlayer()
	{
		return m_Player;
	}

	void Scene::SetPlayer(const unsigned int index)
	{
		m_Player = index;
	}

	void Scene::PointCamera(vec3 position) {
		m_Camera->SetPosition(vec3(position.x, position.y, m_Camera->GetPosition().z));
	}

	std::queue<std::string> Scene::GetDialogueBoxNames() {
		return dialogue_box_names;
	}

	void Scene::PopDialogueBoxNames() {
		dialogue_box_names.pop();
	}

}
