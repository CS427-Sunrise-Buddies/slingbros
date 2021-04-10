#pragma once

#include <yaml-cpp/yaml.h>

// The keys in the yaml file to parse
static const std::string ID_KEY = "id"; // File name
static const std::string LEVEL_NAME_KEY = "name"; // Pretty name
static const std::string BACKGROUND_KEY = "background";
static const std::string BACKGROUND_MUSIC_KEY = "background_music";
static const std::string WEATHER_KEY = "weather";
static const std::string PLAYER_KEY = "player";
static const std::string NUM_PLAYERS_KEY = "num_players";
static const std::string MAP_KEY = "map";
static const std::string DIALOGUE_BOXES_KEY = "dialogue_boxes";
static const std::string ENTITIES_KEY = "entities";
static const std::string LEVELS_KEY = "levels"; // Used in config.yaml

// Per-entity node keys
static const std::string TYPE_KEY = "type";
static const std::string MOTION_KEY = "motion";
static const std::string ANGLE_KEY = "angle";
static const std::string POSITION_KEY = "position";
static const std::string VELOCITY_KEY = "velocity";
static const std::string SCALE_KEY = "scale";
static const std::string CAN_MOVE_KEY = "can_move";
static const std::string TURN_KEY = "turn";
static const std::string ORDER_KEY = "order";
static const std::string SLUNG_KEY = "slung";
static const std::string POINTS_KEY = "points";
static const std::string COUNTDOWN_KEY = "countdown";
static const std::string SIZE_CHANGED_KEY = "size_changed";
static const std::string MASS_KEY = "mass";
static const std::string VALUE_KEY = "value";
static const std::string MASS_CHANGED_KEY = "mass_changed";
static const std::string TURNS_REMAINING_KEY = "turns_remaining";
static const std::string AI_KEY = "ai";
static const std::string TARGET_KEY = "target";

// Entity component type key mapping
static const std::string EMPTY_CELL = "--";
static const std::string T0 = "T0"; // Start tile
static const std::string T1 = "T1"; // Goal tile
static const std::string T2 = "T2"; // Ground tile
static const std::string T3 = "T3"; // Grassy tile
static const std::string T4 = "T4"; // Lava tile
static const std::string T5 = "T5"; // Windy grass tile
static const std::string T6 = "T6"; // Sand tile
static const std::string T7 = "T7"; // Glass tile
static const std::string T8 = "T8"; // Snowy tile
static const std::string T9 = "T9"; // Ice Tile
static const std::string H0 = "H0"; // Hazard tile spike
static const std::string H1 = "H1"; // Hazard ground spike
static const std::string E0 = "E0"; // Basic enemy
static const std::string E1 = "E1"; // Snail enemy
static const std::string E2 = "E2"; // BugDroid enemy
static const std::string E3 = "E3"; // Bird enemy
static const std::string E4 = "E4"; // Blueb enemy
static const std::string E5 = "E5"; // Beehive enemy
static const std::string E6 = "E6"; // Helge enemy
static const std::string P0 = "P0"; // Speed power-up
static const std::string P1 = "P1"; // Size up power-up
static const std::string P2 = "P2"; // Size down power-up
static const std::string P3 = "P3"; // Coin power-up
static const std::string P4 = "P4"; // Mass up power-up
static const std::string S0 = "S0"; // Orange Sling bro
static const std::string S1 = "S1"; // Pink Sling bro
static const std::string X0 = "X0"; // Projectile
static const std::string X1 = "X1"; // Helge projectile

// Weather Mappings
static const std::string SUNNY = "sunny";
static const std::string RAIN = "rain";
static const std::string SNOW = "snow";

// Name of save file
static const std::string SAVE_FILE_NAME = "saved";
static const std::string CONFIG_FILE_NAME = "config";
static const std::string CONFIG2P_FILE_NAME = "config2p";

class LevelManager
{
	public:
		/**
		 * Reads and returns the list of level names from data/levels/config.yaml
		 *
		 * @return List of levels names ordered in the order to be played
		 */
		static std::vector<std::string> get_levels(size_t num_players);

		/**
		 * Convenience method to programmatically generate a level from
		 * a text file and save the level as a Slingbro level.
		 * Should not use this in prod. Instead, use it to generate a level,
		 * copy the generated level into the levels/ directory, and call load_level.
		 *
		 * @param file_name The name of the file containing the level to create
		 */
		static void create_level(const std::string& file_name);

		/**
		 * Loads a Slingbro level from a file into a Scene.
		 *
		 * @param file_path The path to the file containing the level to load
		 * @param camera A pointer to a camera that you want the loaded scene to use as its active camera
		 * @return The created scene of the level
		 */
		static ECS_ENTT::Scene* load_level(const std::string& file_path, Camera* camera);

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

		static WeatherTypes to_weather_type(const std::string s);
		static std::string to_yaml_weather_type(WeatherTypes w);
};
