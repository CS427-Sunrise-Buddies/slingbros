#pragma once

#include "common.hpp"
#include "render_components.hpp"

// Particle system structure based on example done by youtuber The Cherno in https://www.youtube.com/watch?v=GK0jHlv3e3w

struct ParticleProperties
{
	glm::vec3 position;
	glm::vec3 velocity, velocityVariation;
	glm::vec4 colourBegin, colourEnd;
	float sizeBegin, sizeEnd, sizeVariation;
	float lifeTimeMs = DEFAULT_PARTICLE_LIFETIME_MS;
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

	std::vector<Particle> GetParticles() const { return m_ParticlePool; }
	std::vector<Particle> GetActiveParticles() const;
	void clearParticles();

	ShadedMesh* GetParticleMesh() const { return m_ParticleMesh; }
	ShadedMesh* GetParticleMeshInstanced() const { return m_ParticleMeshInstanced; }

private:
	ParticleSystem(uint32_t maxNumParticles);

	static ParticleSystem* instance;
	std::vector<Particle> m_ParticlePool;
	int32 m_PoolIndex;
	ShadedMesh* m_ParticleMesh;
	ShadedMesh* m_ParticleMeshInstanced;
};