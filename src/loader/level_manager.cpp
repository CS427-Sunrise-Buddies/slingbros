#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <common.hpp>
#include <entities/snail_enemy.hpp>
#include <entities/basic_enemy.hpp>
#include <entities/start_tile.hpp>
#include <entities/grassy_tile.hpp>
#include <entities/ground_tile.hpp>
#include <entities/goal_tile.hpp>
#include <entities/lava_tile.hpp>
#include <entities/speed_powerup.hpp>
#include <entities/slingbro.hpp>
#include <entities/projectile.hpp>
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
				{E0, BasicEnemy::createBasicEnemy},
				{E1, SnailEnemy::createSnailEnemy},
				{P0, SpeedPowerUp::createSpeedPowerUp},
				{S0, SlingBro::createSlingBro},
				{X0, Projectile::createProjectile}
		};

// See: https://github.com/jbeder/yaml-cpp/wiki/Tutorial#converting-tofrom-native-data-types
namespace YAML
{
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

	// Tokenize and parse
	printf("Name:\t\t%s\n", level_name.c_str());
	printf("Background:\t%s\n", bg_src.c_str());

	// Map of entities in the level
	auto level_map = file[MAP_KEY].as<std::string>();

	// Split level map into rows
	auto rows = split(level_map, '\n');

	// Create scene with the specified level dimensions
	auto x = split(rows[0], ' ').size();
	auto y = rows.size();
	printf("Scene size:\t(%lu,%lu)\n", x, y);
	auto* scene = new ECS_ENTT::Scene(level_name, {x, y });

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

			// If the current cell is a start tile, create the bros here
			// Assumes that each level only ever has 1 start tile
			if (key == T0)
			{
				SlingBro::createSlingBro(position - glm::vec3(0.0f, 0.0f, 4.0f), scene);
			}
		}
	}

	// Save the created level into the levels directory
	save_level(scene, levels_path(yaml_file(level_name)));
}

ECS_ENTT::Scene *LevelManager::load_level(const std::string& file_path)
{
	// Open the file
	YAML::Node file = YAML::LoadFile(file_path);
	auto level = file[LEVEL_NAME_KEY];
	if (!level) return nullptr;

	// Get name and background of the level
	auto level_name = level.as<std::string>();
	auto bg_src = file[BACKGROUND_KEY].as<std::string>();

	// Map of entities in the level
	auto level_map = file[MAP_KEY].as<std::string>();

	// Split level map into rows
	auto rows = split(level_map, '\n');

	// Create scene with the specified level dimensions
	auto x = split(rows[0], ' ').size();
	auto y = rows.size();
	auto* scene = new ECS_ENTT::Scene(level_name, {x, y });

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

		// Parse entity
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

		//printf("Loaded entity '%s' into scene at (%f,%f)\n", key.c_str(), position.x, position.y);
	}

	return scene;
}

void LevelManager::save_level(ECS_ENTT::Scene* scene)
{
	// todo(atsang): save progress per level
	// Overwrites over old progress
	save_level(scene, saved_path(yaml_file(SAVE_FILE_NAME)));
}

void LevelManager::save_level(ECS_ENTT::Scene *scene, const std::string &file_path)
{
	// Start building level file
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << LEVEL_NAME_KEY;
	out << YAML::Value << scene->m_Name;
	out << YAML::Key << BACKGROUND_KEY;
	out << YAML::Value << "todo"; // TODO: [BROS-36] Parallax

	// Build level map literal scalar
	YAML::Node entities_node;
	std::string map = "";
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

		// Add properties
		if (entity.HasComponent<Motion>())
		{
			auto motion = entity.GetComponent<Motion>();

			YAML::Node motion_node;
			motion_node[ANGLE_KEY] = motion.angle;
			motion_node[POSITION_KEY] = motion.position;
			motion_node[VELOCITY_KEY] = motion.velocity;
			motion_node[SCALE_KEY] = motion.scale;

			entity_node[MOTION_KEY] = motion_node;
		}

		// Add entity node to entities
		entities_node.push_back(entity_node);
	}

	// Emit level map literal scalar
	out << YAML::Key << MAP_KEY;
	out << YAML::Value << YAML::Literal << map;

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
	if (entity.HasComponent<BasicEnemy>()) return E0;
	if (entity.HasComponent<SnailEnemy>()) return E1;
	if (entity.HasComponent<SpeedPowerUp>()) return P0;
	if (entity.HasComponent<SlingBro>()) return S0;
	if (entity.HasComponent<Projectile>()) return X0;
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
