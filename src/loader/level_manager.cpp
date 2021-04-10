#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <queue>

#include <common.hpp>
#include <entities/snail_enemy.hpp>
#include <entities/basic_enemy.hpp>
#include <entities/beehive_enemy.hpp>
#include <entities/bugdroid_enemy.hpp>
#include <entities/bird_enemy.hpp>
#include <entities/blueb_enemy.hpp>
#include <entities/helge_enemy.hpp>
#include <entities/start_tile.hpp>
#include <entities/grassy_tile.hpp>
#include <entities/windy_grass.hpp>
#include <entities/ground_tile.hpp>
#include <entities/goal_tile.hpp>
#include <entities/lava_tile.hpp>
#include <entities/sand_tile.hpp>
#include <entities/glass_tile.hpp>
#include <entities/snowy_tile.hpp>
#include <entities/ice_tile.hpp>
#include <entities/hazard_tile_spike.hpp>
#include <entities/spike_hazard.hpp>
#include <entities/speed_powerup.hpp>
#include <entities/size_up_powerup.hpp>
#include <entities/size_down_powerup.hpp>
#include <entities/coin_powerup.hpp>
#include <entities/mass_up_powerup.hpp>
#include <entities/slingbro.hpp>
#include <entities/projectile.hpp>
#include <entities/helge_projectile.hpp>
#include <entities/parallax_background.hpp>
#include "level_manager.hpp"

typedef ECS_ENTT::Entity (*fn)(vec3, ECS_ENTT::Scene*);
typedef std::map<std::string, fn> FunctionMap;
const FunctionMap fns =
		{
				{T0, StartTile::createStartTile},
				{T1, GoalTile::createGoalTile},
				{T2, GroundTile::createGroundTile},
				{T3, GrassyTile::createGrassyTile},
				{T4, LavaTile::createLavaTile},
				{T5, WindyGrass::createGrass},
				{T6, SandTile::createSandTile},
				{T7, GlassTile::createGlassTile},
				{T8, SnowyTile::createSnowyTile},
				{T9, IceTile::createIceTile},
				{H0, HazardTileSpike::createHazardTileSpike},
				{H1, HazardSpike::createSpikeHazard},
				{E0, BasicEnemy::createBasicEnemy},
				{E1, SnailEnemy::createSnailEnemy},
				{E2, BugDroidEnemy::createBugDroidEnemy},
				{E3, BirdEnemy::createBirdEnemy},
				{E4, BluebEnemy::createBluebEnemy},
				{E5, BeeHiveEnemy::createBeeHiveEnemy},
				{E6, HelgeEnemy::createHelgeEnemy},
				{P0, SpeedPowerUp::createSpeedPowerUp},
				{P1, SizeUpPowerUp::createSizeUpPowerUp},
				{P2, SizeDownPowerUp::createSizeDownPowerUp},
				{P3, CoinPowerUp::createCoinPowerUp},
				{P4, MassUpPowerUp::createMassUpPowerUp},
				{S0, SlingBro::createOrangeSlingBro},
				{S1, SlingBro::createPinkSlingBro},
				{X0, Projectile::createProjectile},
				{X1, HelgeProjectile::createHelgeProjectile}
		};

// See: https://github.com/jbeder/yaml-cpp/wiki/Tutorial#converting-tofrom-native-data-types
namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};
}

std::vector<std::string> LevelManager::get_levels(size_t num_players) {
	// Get the path to the config file holding the list order
	auto file_name = num_players == NUM_PLAYERS_1 ? CONFIG_FILE_NAME : CONFIG2P_FILE_NAME;
	auto file_path = config_path(yaml_file(file_name));

	// Open the file
	YAML::Node file = YAML::LoadFile(file_path);
	auto levels = file[LEVELS_KEY];

	// Must have levels sequence in yaml file
	assert(levels);
	assert(levels.IsSequence());

	// Parse the level names
	std::vector<std::string> names;
	for (auto name : levels)
	{
		names.push_back(name.as<std::string>());
	}
	return names;
}

void LevelManager::create_level(const std::string& file_name)
{
	// Get the path to the file matching file_name
	std::string file_path = create_path(file_name);

	// Open the file
	YAML::Node file = YAML::LoadFile(file_path);
	auto level = file[LEVEL_NAME_KEY];
	if (!level) return;

	// Get name and background of the level
	auto level_name = level.as<std::string>();
	auto bg_src = file[BACKGROUND_KEY].as<std::string>();
	auto bgMusic_src = file[BACKGROUND_MUSIC_KEY].as<std::string>();

	// Tokenize and parse
	printf("Name:\t\t%s\n", level_name.c_str());
	printf("Background:\t%s\n", bg_src.c_str());
	printf("BackgroundMusic:\t%s\n", bgMusic_src.c_str());

	auto weather = file[WEATHER_KEY].as<std::string>();

	// Map of entities in the level
	auto level_map = file[MAP_KEY].as<std::string>();

	// Split level map into rows
	auto rows = split(level_map, '\n');

	// Create scene with the specified level dimensions
	auto x = split(rows[0], ' ').size();
	auto y = rows.size();
	printf("Scene size:\t(%lu,%lu)\n", x, y);
	auto* scene = new ECS_ENTT::Scene(level_name, {x, y});

	// Set scene id which is really just the file name minus extension
	// This is used to get the level index in world.cpp
	scene->m_Id = split(file_name, '.')[0];
	scene->m_Weather = to_weather_type(weather);

	// Set background in scene
	if (!bg_src.empty())
		ParallaxBackground::createBackground(scene, bg_src);

	// Set background music
	scene->m_BackgroundMusicFileName = bgMusic_src;

	auto dialogues = file[DIALOGUE_BOXES_KEY];
	if (dialogues)
	{
		std::queue<std::string> dialogue_boxes;
		for (auto dialogue : dialogues)
		{
			dialogue_boxes.push(dialogue.as<std::string>());
		}
		scene->dialogue_box_names = dialogue_boxes;
	}

	for (int i = 0; i < rows.size(); i++) // Row index
	{
		// Convert each key to entity in the row and add to scene graph and registry
		auto keys = split(rows[i], ' ');
		for (int j = 0; j < keys.size(); j++) // Column index
		{
			// Extract the key in the level map in the yaml
			auto key = keys[j];
			scene->m_Map[i][j] = key;
			if (key == EMPTY_CELL) continue;

			// Calculate position
			vec3 position = vec3(j * SPRITE_SCALE, i * SPRITE_SCALE, 0.f);

			// Map the key in the level map to the entity create function and create the entity
			const auto& create_entity = fns.at(key);
			(*create_entity)(position, scene);

			//printf("Created entity '%s' in scene at (%f,%f)\n", key.c_str(), position.x, position.y);
		}
	}

	// First player starts by default
	scene->SetPlayer(0);

	// Save the created level into the repository's build levels directory
	save_level(scene, levels_path(file_name));
}

ECS_ENTT::Scene *LevelManager::load_level(const std::string& file_path, Camera* camera)
{
	// Open the file
	YAML::Node file = YAML::LoadFile(file_path);
	auto level = file[LEVEL_NAME_KEY];
	if (!level) return nullptr;

	// Get name and background of the level
	auto level_name = level.as<std::string>();
	auto bg_src = file[BACKGROUND_KEY].as<std::string>();
	auto bgMusic_src = file[BACKGROUND_MUSIC_KEY].as<std::string>();

	auto weather = file[WEATHER_KEY].as<std::string>();

	// Map of entities in the level
	auto level_map = file[MAP_KEY].as<std::string>();

	// Split level map into rows
	auto rows = split(level_map, '\n');

	// Create scene with the specified level dimensions
	auto x = split(rows[0], ' ').size();
	auto y = rows.size();
	auto* scene = new ECS_ENTT::Scene(level_name, {x, y}, camera);

	// Set scene id which is really just the file name minus extension
	// This is used to get the level index in world.cpp
	scene->m_Id = file[ID_KEY].as<std::string>();
	scene->m_Weather = to_weather_type(weather);

	// Map of dialogue boxes for level
	auto level_dialogue_boxes = file[DIALOGUE_BOXES_KEY];
	std::queue<std::string> dialogue_boxes_queue;
	for (auto dialogue_src : level_dialogue_boxes)
	{
		dialogue_boxes_queue.push(dialogue_src.as<std::string>());
	}
	scene->dialogue_box_names = dialogue_boxes_queue;
	scene->is_in_dialogue = false;
	
	// Current player whose turn it is
	auto player_idx = file[PLAYER_KEY].as<unsigned int>();
	scene->SetPlayer(player_idx);

	// Number of players (or skip if loading new level)
	if (file[NUM_PLAYERS_KEY])
	{
		auto num_players = file[NUM_PLAYERS_KEY].as<size_t>();
		scene->SetNumPlayer(num_players);
	}

	// Convert level map in yaml to 2d vector
	for (int i = 0; i < rows.size(); i++) // Row index
	{
		auto keys = split(rows[i], ' ');
		for (int j = 0; j < keys.size(); j++) // Column index
		{
			scene->m_Map[i][j] = keys[j];
		}
	}

	// Load entities
	auto entities = file[ENTITIES_KEY];
	for (auto entity : entities)
	{
		// Get entity type key
		auto key = entity[TYPE_KEY].as<std::string>();

		// Parse motion component
		auto motion = entity[MOTION_KEY];
		auto position = motion[POSITION_KEY].as<glm::vec3>();
		auto angle = motion[ANGLE_KEY].as<float>();
		auto velocity = motion[VELOCITY_KEY].as<glm::vec3>();
		auto scale = motion[SCALE_KEY].as<glm::vec3>();

		// Map the key in the level map to the entity create function and create the entity
		const auto& create_entity = fns.at(key);
		auto e = (*create_entity)(position, scene);

		// Set motion component
		auto& e_motion = e.GetComponent<Motion>();
		e_motion.angle = angle;
		e_motion.velocity = velocity;
		e_motion.scale = scale;

		// Check if turn component exists
		auto turn = entity[TURN_KEY];
		if (turn)
		{
			// Parse turn component
			auto order = turn[ORDER_KEY].as<unsigned int>();
			auto slung = turn[SLUNG_KEY].as<bool>();
			auto points = turn[POINTS_KEY].as<int>();
			auto countdown = turn[COUNTDOWN_KEY].as<float>();

			// Set turn component
			auto& e_turn = e.GetComponent<Turn>();
			e_turn.order = order;
			e_turn.slung = slung;
			e_turn.points = points;
			e_turn.countdown = countdown;
		}

		// Check if sizeChanged component exists
		auto size_changed = entity[SIZE_CHANGED_KEY];
		if (size_changed)
		{
			// Parse size changed component
			auto turns_remaining = size_changed[TURNS_REMAINING_KEY].as<int>();

			// Set size changed component
			auto& e_size_changed = e.AddComponent<SizeChanged>();
			e_size_changed.turnsRemaining = turns_remaining;
		}

		// Check if mass component exists
		auto mass = entity[MASS_KEY];
		if (mass)
		{
			// Parse value component
			auto value = mass[VALUE_KEY].as<float>();

			auto& e_mass = e.GetComponent<Mass>();
			e_mass.value = value;
		}

		// Check if massChanged component exists
		auto mass_changed = entity[MASS_CHANGED_KEY];
		if (mass_changed)
		{
			// Parse mass changed component
			auto turns_remaining = mass_changed[TURNS_REMAINING_KEY].as<int>();

			// Set mass changed component
			auto& e_mass_changed = e.AddComponent<MassChanged>();
			e_mass_changed.turnsRemaining = turns_remaining;
		}

		// Check if AI component exist
		auto ai = entity[AI_KEY];
		if (ai)
		{
			// Enemy entities should be created with the AI component
			auto& e_ai = e.HasComponent<AI>() ? e.GetComponent<AI>() : e.AddComponent<AI>();
			e_ai.countdown = ai[COUNTDOWN_KEY].as<float>();

			// Target is std::optional so we check existence before parsing and setting
			if (ai[TARGET_KEY])
			{
				e_ai.target = ai[TARGET_KEY].as<glm::vec2>();
			}
		}

		//printf("Loaded entity '%s' into scene at (%f,%f)\n", key.c_str(), position.x, position.y);
	}

	// Add a background to the loaded scene based on filename in yaml file
	if (!bg_src.empty()) // ignore if no background image has been set
		ParallaxBackground::createBackground(scene, bg_src);

	// Load background music into scene.
	scene->m_BackgroundMusicFileName = bgMusic_src;

	return scene;
}

void LevelManager::save_level(ECS_ENTT::Scene* scene)
{
	// Overwrites over old progress
	save_level(scene, saved_path(yaml_file(SAVE_FILE_NAME)));
}

void LevelManager::save_level(ECS_ENTT::Scene *scene, const std::string &file_path)
{
	// Start building level file
	YAML::Emitter out;
	out << YAML::BeginMap;

	// File name
	out << YAML::Key << ID_KEY;
	out << YAML::Value << scene->m_Id;

	// Name of level
	out << YAML::Key << LEVEL_NAME_KEY;
	out << YAML::Value << scene->m_Name;

	// File name of background image
	out << YAML::Key << BACKGROUND_KEY;
	out << YAML::Value << scene->m_BackgroundFilename;

	// File name of weather
	out << YAML::Key << WEATHER_KEY;
	out << YAML::Value << to_yaml_weather_type(scene->m_Weather);

	// File name of background music
	out << YAML::Key << BACKGROUND_MUSIC_KEY;
	out << YAML::Value << scene->m_BackgroundMusicFileName;

	// Player index whose turn it is
	out << YAML::Key << PLAYER_KEY;
	out << YAML::Value << scene->GetPlayer();

	// Number of players (or skip if creating level)
	if (scene->GetNumPlayers() > 0)
	{
		out << YAML::Key << NUM_PLAYERS_KEY;
		out << YAML::Value << scene->GetNumPlayers();
	}

	// Build level map literal scalar
	YAML::Node entities_node;
	std::string map;
	int num_rows = scene->m_Map.size();
	for (int i = 0; i < num_rows; i++)
	{
		auto row = scene->m_Map[i];
		for (const auto& entity_type : row)
		{
			// Add space
			map += " " + entity_type;
		}

		// Add new line if not at end
		if (i < num_rows - 1) map += "\n";
	}

	// Build entities sequence
	auto view = scene->m_Registry.view<Motion>();
	for (auto id : view)
	{
		// Retrieve the entity
		auto entity = ECS_ENTT::Entity(id, scene);

		// Create YAML node to hold current entity
		YAML::Node entity_node;
		entity_node[TYPE_KEY] = to_type_key(entity);

		// Don't save this entity
		if (entity.HasComponent<IgnoreSave>())
			continue;

		// Add motion properties
		if (entity.HasComponent<Motion>())
		{
			auto motion = entity.GetComponent<Motion>();

			YAML::Node motion_node;
			motion_node[ANGLE_KEY] = motion.angle;
			motion_node[POSITION_KEY] = motion.position;
			motion_node[VELOCITY_KEY] = motion.velocity;
			motion_node[SCALE_KEY] = motion.scale;
			motion_node[CAN_MOVE_KEY] = motion.can_move;

			entity_node[MOTION_KEY] = motion_node;
		}

		// Add turn properties
		if (entity.HasComponent<Turn>())
		{
			auto turn = entity.GetComponent<Turn>();

			YAML::Node turn_node;
			turn_node[ORDER_KEY] = turn.order;
			turn_node[SLUNG_KEY] = turn.slung;
			turn_node[POINTS_KEY] = turn.points;
			turn_node[COUNTDOWN_KEY] = turn.countdown;

			entity_node[TURN_KEY] = turn_node;
		}

		// Add sizeChanged properties
		if (entity.HasComponent<SizeChanged>())
		{
			auto size_changed = entity.GetComponent<SizeChanged>();

			YAML::Node size_changed_node;
			size_changed_node[TURNS_REMAINING_KEY] = size_changed.turnsRemaining;

			entity_node[SIZE_CHANGED_KEY] = size_changed_node;
		}

		// Add mass properties
		if (entity.HasComponent<Mass>())
		{
			auto mass = entity.GetComponent<Mass>();

			YAML::Node mass_node;
			mass_node[VALUE_KEY] = mass.value;
			entity_node[MASS_KEY] = mass_node;
		}

		// Add massChanged properties
		if (entity.HasComponent<MassChanged>())
		{
			auto mass_changed = entity.GetComponent<MassChanged>();

			YAML::Node mass_changed_node;
			mass_changed_node[TURNS_REMAINING_KEY] = mass_changed.turnsRemaining;

			entity_node[MASS_CHANGED_KEY] = mass_changed_node;
		}

		// Add AI properties
		if (entity.HasComponent<AI>())
		{
			auto ai = entity.GetComponent<AI>();

			YAML::Node ai_node;
			ai_node[COUNTDOWN_KEY] = ai.countdown;

			if (ai.target)
			{
				ai_node[TARGET_KEY] = ai.target.value();
			}

			entity_node[AI_KEY] = ai_node;
		}

		// Add entity node to entities
		entities_node.push_back(entity_node);
	}

	// Emit level map literal scalar
	out << YAML::Key << MAP_KEY;
	out << YAML::Value << YAML::Literal << map;
	
	std::queue<std::string> names = scene->GetDialogueBoxNames();

	if (!names.empty())
	{
		YAML::Node dialogue_boxes_node;

		// Save current dialogue box if exists
		if (!scene->current_dialogue_box.empty())
		{
			YAML::Node dialogue_box_node;
			dialogue_boxes_node.push_back(scene->current_dialogue_box);
		}

		while (!names.empty())
		{
			YAML::Node dialogue_box_node;
			dialogue_boxes_node.push_back(names.front());
			names.pop();
		}

		out << YAML::Key << DIALOGUE_BOXES_KEY;
		out << YAML::Value << dialogue_boxes_node;
	}

	// Emit entities sequence
	out << YAML::Key << ENTITIES_KEY;
	out << YAML::Value << entities_node;

	// End map
	out << YAML::EndMap;

	// Save to file
	std::ofstream fout(file_path);
	fout << out.c_str();
	fout.close();
	printf("Saved to %s\n", file_path.c_str());
}

std::string LevelManager::to_type_key(ECS_ENTT::Entity entity) {
	if (entity.HasComponent<StartTile>()) return T0;
	if (entity.HasComponent<GoalTile>()) return T1;
	if (entity.HasComponent<GroundTile>()) return T2;
	if (entity.HasComponent<GrassyTile>()) return T3;
	if (entity.HasComponent<LavaTile>()) return T4;
	if (entity.HasComponent<WindyGrass>()) return T5;
	if (entity.HasComponent<SandTile>()) return T6;
	if (entity.HasComponent<GlassTile>()) return T7;
	if (entity.HasComponent<SnowyTile>()) return T8;
	if (entity.HasComponent<IceTile>()) return T9;
	if (entity.HasComponent<HazardTileSpike>()) return H0;
	if (entity.HasComponent<HazardSpike>()) return H1;
	if (entity.HasComponent<BasicEnemy>()) return E0;
	if (entity.HasComponent<SnailEnemy>()) return E1;
	if (entity.HasComponent<BugDroidEnemy>()) return E2;
	if (entity.HasComponent<BirdEnemy>()) return E3;
	if (entity.HasComponent<BluebEnemy>()) return E4;
	if (entity.HasComponent<BeeHiveEnemy>()) return E5;
	if (entity.HasComponent<HelgeEnemy>()) return E6;
	if (entity.HasComponent<SpeedPowerUp>()) return P0;
	if (entity.HasComponent<SizeUpPowerUp>()) return P1;
	if (entity.HasComponent<SizeDownPowerUp>()) return P2;
	if (entity.HasComponent<CoinPowerUp>()) return P3;
	if (entity.HasComponent<MassUpPowerUp>()) return P4;
	if (entity.HasComponent<OrangeBro>()) return S0;
	if (entity.HasComponent<PinkBro>()) return S1;
	if (entity.HasComponent<Projectile>()) return X0;
	if (entity.HasComponent<HelgeProjectile>()) return X1;
	printf("Unaccounted for component type\n");
	return T2; // Default: GroundTile
}

std::vector<std::string> LevelManager::split(const std::string &in, char delim)
{
	std::istringstream iss(in);
	std::string token;
	std::vector<std::string> tokens;
	auto inserter = std::back_inserter(tokens);
	while (std::getline(iss, token, delim))
	{
		// Skip empty tokens
		if (!token.empty())
		{
			*inserter++ = token;
		}
	}
	return tokens;
}

WeatherTypes LevelManager::to_weather_type(const std::string s)
{
	if (s == SUNNY) return WeatherTypes::Sunny;
	if (s == RAIN) return WeatherTypes::Rain;
	if (s == SNOW) return WeatherTypes::Snow;
	return WeatherTypes::Sunny;
}

std::string LevelManager::to_yaml_weather_type(WeatherTypes w)
{
	if (w == WeatherTypes::Sunny) return SUNNY;
	if (w == WeatherTypes::Rain) return RAIN;
	if (w == WeatherTypes::Snow) return SNOW;
	return SUNNY;
}
