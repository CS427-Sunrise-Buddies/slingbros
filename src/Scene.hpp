#pragma once

#include "entt.hpp"

// Credit for the general Entity/Scene implementation structure used here goes to The Cherno, see reference video here: https://www.youtube.com/watch?v=D4hz0wEB978&list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT&index=77

class Entity;

// Class to be used to manage Entities in a structured way. Every Scene object will have its own registry that contains all of the Entities in the Scene.
class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	static std::shared_ptr<Scene> Create() { return std::make_shared<Scene>(); }

	Entity CreateEntity(const std::string& name = std::string());
	void DestroyEntity(Entity entity);

	void OnUpdate(float deltaTime);

private:
	template<typename T>
	void OnComponentAdded(Entity entity, T& component);

private:
	friend class Entity;

	// Registry to contain the component data and entity IDs
	entt::registry m_Registry;

	// TODO: these will need to be set properly whenever the game window is created/resized
	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
};

