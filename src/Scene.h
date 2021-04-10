#pragma once

#include <string>
#include <optional>
#include "entt.hpp"
#include "Camera.h"
#include <queue>
#include "weather.hpp"

#include <glm/vec2.hpp>

// Credit for the general Entity/Scene implementation structure used here goes to The Cherno, see reference video here: https://www.youtube.com/watch?v=D4hz0wEB978&list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT&index=77

namespace ECS_ENTT
{

	class Entity;

	// Class to be used to manage Entities in a structured way. Every Scene object will have its own registry that contains all of the Entities in the Scene.
	class Scene
	{
	public:
		Scene() = default;

		explicit Scene(std::string name, glm::vec2 size);
		explicit Scene(std::string name, glm::vec2 size, Camera* camera);

		~Scene() = default;

		Entity CreateEntity(const std::string& name = std::string());

		void DestroyEntity(Entity entity);

		unsigned int GetPlayer();

		void SetPlayer(unsigned int index);

		size_t GetNumPlayers();

		void SetNumPlayer(size_t n);

		Camera* GetCamera() const { return m_Camera; }

		void PointCamera(glm::vec3 position);
		
		std::queue<std::string> GetDialogueBoxNames();
		
		void PopDialogueBoxNames();
		
	public:
		// Unique identifier of the scene
		// Should just be the name of the level file it corresponds to
		std::string m_Id;

		// Name of the scene
		std::string m_Name;

		// Name of the scene's background file
		std::string m_BackgroundFilename;

		// Registry to contain the component data and entity IDs
		entt::registry m_Registry;

		// Size of the scene
		glm::vec2 m_Size;

		// Each scene has a camera
		Camera* m_Camera;

		// Index of current player
		unsigned int m_Player = 0;

		// Number of players
		size_t m_NumPlayers = 0;
		
		// File names of Dialogue boxes
		std::queue<std::string> dialogue_box_names;
				
		bool is_in_dialogue;
		
		std::string current_dialogue_box;

		// Grid of entity type keys for AI path finding
		typedef std::vector<std::string> LevelRow;
		typedef std::vector<LevelRow> LevelMap;
		LevelMap m_Map;

		// Name of the background music used for this scene
		std::string m_BackgroundMusicFileName;

		WeatherTypes m_Weather;

	private:
		friend class Entity;
	};

}
