#pragma once

#include "common.hpp"
#include "render_components.hpp"
#include "weather.hpp"

#include <random>

// Particle system structure based on example done by youtuber The Cherno in https://www.youtube.com/watch?v=GK0jHlv3e3w

class Random
{
public:
	static void Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	static float Float()
	{
		return (float)s_Distribution(s_RandomEngine) / 100.f;
	}

private:
	static std::mt19937 s_RandomEngine;
	static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};

struct ParticleProperties
{
	glm::vec3 position;
	glm::vec3 velocity, velocityVariation;
	glm::vec4 colourBegin, colourEnd;
	float sizeBegin, sizeEnd, sizeVariation;
	float lifeTimeMs = DEFAULT_PARTICLE_LIFETIME_MS;
	bool affectedByWind = false;
};

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 colourBegin, colourEnd, currentColour;
	float rotation = 0.0f;
	float sizeBegin, sizeEnd, currentSize;

	float lifeTimeMs = DEFAULT_PARTICLE_LIFETIME_MS;
	float lifeRemaining = 0.0f;

	bool active = false;
	bool affectedByWind = false;
};

struct Bee
{
	glm::vec3 position;
	glm::vec3 offsetFromSwarmCenter;
	glm::vec3 velocity;
	float rotationX = 0.0f;
	float rotationY = 0.0f;
	float rotationZ = 0.0f;
	float size = 1.0f;
	glm::vec4 colour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // default bee colour is yellow
};

struct BeeSwarm
{
	BeeSwarm(glm::vec3 swarmCenterPosition, unsigned int numberOfBees);
	glm::vec3 position;
	unsigned int numBees;
	std::vector<Bee> bees;

	bool isChasing = false;
	uint32_t targetedPlayerEntityID = -1;
	ECS_ENTT::Scene* scene = nullptr;
};

class ParticleSystem
{
public:
	static ParticleSystem* GetInstance();

	void step(float elapsed_ms);

	void Emit(const ParticleProperties& particleProps);

	void grass_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall);
	void dirt_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall);
	void lava_block_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall);
	void beehive_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall);

	void weather_listener(ECS_ENTT::Scene* scene);

	std::vector<Particle> GetParticles() const { return m_ParticlePool; }
	std::vector<Particle> GetActiveParticles() const;
	void clearParticles();
	void clearBeeSwarms();
	void updateBeeProperties(Bee& bee, glm::vec3 swarmPosition);

	ShadedMesh* GetParticleMesh() const { return m_ParticleMesh; }
	ShadedMesh* GetParticleMeshInstanced() const { return m_ParticleMeshInstanced; }

	ShadedMesh* GetBeeMesh() const { return m_BeeMesh; }
	std::vector<BeeSwarm*>& GetBeeSwarms() { return m_BeeSwarms; }
	
	BeeSwarm* CreateBeeSwarm(glm::vec3 swarmCenterPosition, unsigned int numberOfBees);
	uint32_t NumBeesTargetingEntity(uint32_t entityID);

private:
	ParticleSystem(uint32_t maxNumParticles);

	static ParticleSystem* instance;
	std::vector<Particle> m_ParticlePool;
	int32 m_PoolIndex;
	ShadedMesh* m_ParticleMesh;
	ShadedMesh* m_ParticleMeshInstanced;
	ShadedMesh* m_BeeMesh;

	std::vector<BeeSwarm*> m_BeeSwarms;
};