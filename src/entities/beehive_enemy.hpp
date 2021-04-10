#include "common.hpp"
#include "Entity.h"
#include "Scene.h"
#include "particle_system.hpp"

struct BeeHiveEnemy {

	static ECS_ENTT::Entity createBeeHiveEnemy(vec3 position, ECS_ENTT::Scene* scene);
	bool HasBeenHarvestedByPlayer(uint32_t entityID) const;

	BeeSwarm* hiveSwarm = nullptr;
	std::vector<uint32_t> harvestedByPlayers;


};