#pragma once

#include "entt.hpp"
#include "Scene.hpp"

// Credit for the general Entity/Scene implementation structure used here goes to The Cherno, see reference video here: https://www.youtube.com/watch?v=D4hz0wEB978&list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT&index=77

namespace ECS_ENTT {

	class Entity
	{
	public:
		Entity() = default;
		Entity(const entt::entity entityID, Scene* scenePtr)
			: m_EntityID(entityID),
			m_Scene(scenePtr) {}
		Entity(const Entity& otherEntity) = default;
		~Entity() = default;

		template<typename T>
		T& GetComponent()
		{
			assert(HasComponent<T>(), "Entity does not have this type of component!");
			return m_Scene->m_Registry.get<T>(m_EntityID);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			assert(!HasComponent<T>(), "Trying to add a component that this Entity already has!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityID, std::forward<Args>(args)...);
			// TODO (once we actually have components!)
			// Easy way to handle anything that needs to happen when a certain component is added:
			//m_Scene->OnComponentAdded<T>(*this, component); 
			return component;
		}

		template<typename T>
		void RemoveComponent()
		{
			assert(HasComponent<T>(), "Entity does not have this type of component!");
			m_Scene->m_Registry.remove<T>(m_EntityID);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityID);
		}

		// Operator overloads
		bool operator==(const Entity& otherEntity) {
			return (this->m_EntityID == otherEntity.m_EntityID)
				&& (this->m_Scene == otherEntity.m_Scene);
		}
		bool operator!=(const Entity& otherEntity) {
			return !operator==(otherEntity);
		}
		operator uint32_t() const { return (uint32_t)m_EntityID; }
		operator entt::entity() const { return (entt::entity)m_EntityID; }
		operator bool() const { return (uint32_t)m_EntityID != entt::null; }

	private:
		entt::entity m_EntityID{ entt::null };
		Scene* m_Scene = nullptr;
	};
}