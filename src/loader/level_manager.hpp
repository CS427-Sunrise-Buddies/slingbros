#pragma once

#include <yaml-cpp/yaml.h>

// The keys in the yaml file to parse
static const std::string LEVEL_NAME_KEY = "name";
static const std::string BACKGROUND_KEY = "background";
static const std::string MAP_KEY = "map";
static const std::string ENTITIES_KEY = "entities";

// Per-entity node keys
static const std::string ID_KEY = "id";
static const std::string TYPE_KEY = "type";
static const std::string MOTION_KEY = "motion";
static const std::string ANGLE_KEY = "angle";
static const std::string POSITION_KEY = "position";
static const std::string VELOCITY_KEY = "velocity";
static const std::string SCALE_KEY = "scale";

// Entity component type key mapping
static const std::string EMPTY_CELL = "--";
static const std::string T0 = "T0"; // Start tile
static const std::string T1 = "T1"; // Goal tile
static const std::string T2 = "T2"; // Ground tile
static const std::string T3 = "T3"; // Grassy tile
static const std::string T4 = "T4"; // Lava tile
static const std::string E0 = "E0"; // Basic enemy
static const std::string E1 = "E1"; // Snail enemy
static const std::string P0 = "P0"; // Speed power-up
static const std::string S0 = "S0"; // Sling bro
static const std::string X0 = "X0"; // Projectile

// Name of save file
static const std::string SAVE_FILE_NAME = "saved";

class LevelManager
{
	public:
		/**
		 * Convenience method to programmatically generate a level from
		 * a text file and save the level as a Slingbro level.
		 *
		 * @param file_name The name of the file containing the level to create
		 */
		static void create_level(const std::string& file_name);

		/**
		 * Loads a Slingbro level from a file into a Scene.
		 *
		 * @param file_path The path to the file containing the level to load
		 * @return The created scene of the level
		 */
		static ECS_ENTT::Scene* load_level(const std::string& file_path);

		/**
		 * Saves a scene into a Slingbro level yaml file to load in the future.
		 * Used to save user level progress.
		 *
		 * @param scene The scene to save
		 */
		static void save_level(ECS_ENTT::Scene* scene);

	private:
		/**
		 * Saves a scene into a Slingbro level yaml file to load in the future
		 * at the specified path. Used to save created levels.
		 *
		 * @param scene The scene to save
		 * @param file_path The path to save the yaml file
		 */
		static void save_level(ECS_ENTT::Scene* scene, const std::string& file_path);

		/**
		 * Maps the given entity to a level map key
		 *
		 * @param entity The entity to get the key for
		 * @return The level map key for the entity type
		 */
		static std::string to_type_key(ECS_ENTT::Entity entity);

		/**
		 * Splits a string into a vector by the provided delimiter.
		 * See: https://stackoverflow.com/a/236803
		 *
		 * @param in The input string
		 * @param delim The character to split the string by
		 * @return A vector containing the split parts of the string
		 */
		static std::vector<std::string> split(const std::string &in, char delim);
};
