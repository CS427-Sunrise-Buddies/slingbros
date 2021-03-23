#include "particle_system.hpp"
#include "entities/windy_grass.hpp"
#include "entities/lava_tile.hpp"
#include "entities/ground_tile.hpp"
#include "entities/grassy_tile.hpp"
#include "entities/slingbro.hpp"
#include "entities/speed_powerup.hpp"
#include "render.hpp"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

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
		return (float) s_Distribution(s_RandomEngine) / 100.f;
	}

private:
	static std::mt19937 s_RandomEngine;
	static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};

std::mt19937 Random::s_RandomEngine;
std::uniform_int_distribution<std::mt19937::result_type> Random::s_Distribution(1, 100);

ParticleSystem* ParticleSystem::instance = nullptr;

ParticleSystem::ParticleSystem(uint32_t maxNumParticles)
	: m_PoolIndex(maxNumParticles - 1), m_ParticleMesh(nullptr), m_ParticleMeshInstanced(nullptr)
{
	Random::Init();
	m_ParticlePool.resize(maxNumParticles);
	
	std::string key = "particleSystemShadedMesh";
	ShadedMesh* particleMesh = &cache_resource(key);

	if (particleMesh->effect.program.resource == 0) {
		RenderSystem::createParticle(*particleMesh, "particle_shader");
	}

	std::string keyInstanced = "particleSystemShadedMeshInstanced";
	ShadedMesh* particleMeshInstanced = &cache_resource(keyInstanced);

	if (particleMeshInstanced->effect.program.resource == 0) {
		RenderSystem::createParticle(*particleMeshInstanced, "particle_shader_instanced");
	}
	
	// Store references to the potentially re-used mesh objects
	m_ParticleMesh = particleMesh;
	m_ParticleMeshInstanced = particleMeshInstanced;
}

ParticleSystem* ParticleSystem::GetInstance()
{
	if (!instance)
		instance = new ParticleSystem(MAX_NUM_PARTICLES);
	
	return instance;
}

void ParticleSystem::step(float elapsed_ms) 
{
	for (auto& particle : m_ParticlePool)
	{
		if (!particle.active)
			continue;

		if (particle.lifeRemaining <= 0.0f)
		{
			particle.active = false;
			particle.currentColour = glm::vec4(0.0f);
			continue;
		}

		particle.lifeRemaining -= elapsed_ms;
		particle.position += particle.velocity * elapsed_ms;
		particle.rotation += elapsed_ms/1000.0f;

		// interpolate particle colour and size between set parameters
		float lifeSpan = particle.lifeRemaining / particle.lifeTimeMs;
		particle.currentColour = glm::lerp(particle.colourEnd, particle.colourBegin, lifeSpan);
		particle.currentSize = glm::lerp(particle.sizeEnd, particle.sizeBegin, lifeSpan);
	}
}

void ParticleSystem::Emit(const ParticleProperties& particleProps)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.active = true;
	particle.position = particleProps.position;
	particle.rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	particle.velocity = particleProps.velocity;
	particle.velocity.x += particleProps.velocityVariation.x * (Random::Float() - 0.5f);
	particle.velocity.y += particleProps.velocityVariation.y * (Random::Float() - 0.5f);
	particle.velocity.z += particleProps.velocityVariation.z * (Random::Float() - 0.5f);

	// Color
	particle.colourBegin = particleProps.colourBegin;
	particle.colourEnd = particleProps.colourEnd;
	particle.currentColour = particle.colourBegin;

	particle.lifeTimeMs = particleProps.lifeTimeMs;
	particle.lifeRemaining = particleProps.lifeTimeMs;
	particle.sizeBegin = particleProps.sizeBegin + particleProps.sizeVariation * (Random::Float() - 0.5f);
	particle.sizeEnd = particleProps.sizeEnd;
	particle.currentSize = particle.sizeBegin;

	m_PoolIndex--;
	if(m_PoolIndex < 0)
		m_PoolIndex = MAX_NUM_PARTICLES - 1;
}

// Collisions between between bro and windy grass tiles - callback function, listening to PhysicsSystem::Collisions
void ParticleSystem::grass_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall)
{
	if (!entity_i.IsValid() || !entity_j.IsValid())
		return;

	if (entity_j.HasComponent<WindyGrass>() && entity_i.HasComponent<SlingBro>())
	{
		Motion& grassMotionComponent = entity_j.GetComponent<Motion>();
		Motion& slingBroMotionComponent = entity_i.GetComponent<Motion>();
		slingBroMotionComponent.velocity *= 0.99;

		if (glm::length(slingBroMotionComponent.velocity) < 100.0f || grassMotionComponent.position.y >= slingBroMotionComponent.position.y + 20.0f)
			return;

		// Emit grass particle going downward
		ParticleProperties particle;
		glm::vec3 particleOffset = glm::vec3(0.0f, 40.0f, 10.0f);
		particle.position = slingBroMotionComponent.position + particleOffset;
		particle.velocity = glm::vec3(0.0f, 0.05f, 0.0f);
		particle.velocityVariation = glm::vec3(0.5f, 0.2f, 0.0f);
		particle.colourBegin = glm::vec4(0.0f, 1.0f, 0.2f, 0.8f);
		particle.colourEnd = glm::vec4(0.0f, 0.4f, 0.0f, 1.0f);
		particle.sizeBegin = 10.0f;
		particle.sizeEnd = 2.0f;
		particle.sizeVariation = 5.0f;
		particle.lifeTimeMs = 1000.0f;
		for (int i = 0; i < 1; i++)
			Emit(particle);

		// Emit grass particle with slight upward velocity
		particle.velocity = glm::vec3(0.0f, -0.025f, 0.0f);
		particle.velocityVariation = glm::vec3(0.1f, 0.05f, 0.0f);
		particle.colourBegin = glm::vec4(0.0f, 0.6f, 0.1f, 0.8f);
		particle.colourEnd = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		particle.sizeBegin = 5.0f;
		particle.sizeEnd = 12.0f;
		particle.sizeVariation = 5.0f;
		particle.lifeTimeMs = 3000.0f;
		for (int i = 0; i < 2; i++)
			Emit(particle);

		// Emit dirt particles 
		particle.position -= glm::vec3(0.0f, 0.0f, 1.0f);
		particle.velocity = glm::vec3(0.0f, -0.1f, 0.025f);
		particle.velocityVariation = glm::vec3(0.5f, 0.1f, 0.0f);
		particle.colourBegin = glm::vec4(105.0f / 255.0f, 56.0f / 255.0f, 25.0f / 255.0f, 0.0f);
		particle.colourEnd = glm::vec4(105.0f / 255.0f, 56.0f / 255.0f, 25.0f / 255.0f, 1.0f);
		particle.sizeBegin = 4.0f;
		particle.sizeEnd = 1.0f;
		particle.sizeVariation = 1.0f;
		particle.lifeTimeMs = 200.0f;
		for (int i = 0; i < 1; i++)
			Emit(particle);
		particle.sizeBegin = 8.0f;
		particle.sizeEnd = 4.0f;
		for (int i = 0; i < 1; i++)
			Emit(particle);
	}

}

// Collisions between between bro and windy grass tiles - callback function, listening to PhysicsSystem::Collisions
void ParticleSystem::dirt_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall)
{
	if (!entity_i.IsValid() || !entity_j.IsValid())
		return;

	if ((entity_j.HasComponent<GroundTile>() || entity_j.HasComponent<GrassyTile>()) && entity_i.HasComponent<SlingBro>())
	{
		Motion& slingBroMotionComponent = entity_i.GetComponent<Motion>();
		Motion& tileMotionComponent = entity_j.GetComponent<Motion>();

		float slingBroSpeed = glm::length(slingBroMotionComponent.velocity);
		if (slingBroSpeed < 100.0f || abs(slingBroMotionComponent.velocity.y) < 20.0f)
			return;

		// Check orientation of character from block and set particle offset correctly
		glm::vec3 dispVec = slingBroMotionComponent.position - tileMotionComponent.position;
		float angle = atan(dispVec.y, dispVec.x); // angle in radians from block to slingbro
		glm::vec3 particleOffset = glm::vec3(0.0f);
		glm::vec3 dirtVelocity = glm::vec3(1.0f);
		float dirtSpeed = 0.1f;
		float pi = 3.1415926535;
		float offsetMagnitudeX = slingBroMotionComponent.scale.x / 2.0f;
		float offsetMagnitudeY = slingBroMotionComponent.scale.y / 2.0f;
		if (angle > -pi / 4 && angle <= pi / 4) // bro to the right of block
		{
			particleOffset = glm::vec3(-offsetMagnitudeX, 0.0f, 10.0f);
			dirtVelocity = glm::vec3(dirtSpeed, 0.0f, 0.0f);
		}
		else if (angle > pi / 4 && angle <= 3 * pi / 4) // bro below block
		{
			particleOffset = glm::vec3(0.0f, -offsetMagnitudeY, 10.0f);
			dirtVelocity = glm::vec3(0.0f, dirtSpeed, 0.0f);
		}
		else if (angle > 3 * pi / 4 && angle <= 5 * pi / 4) // bro to the left of block
		{
			particleOffset = glm::vec3(offsetMagnitudeX, 0.0f, 10.0f);
			dirtVelocity = glm::vec3(-dirtSpeed, 0.0f, 0.0f);
		}
		else // bro above block
		{
			particleOffset = glm::vec3(0.0f, offsetMagnitudeY, 10.0f);
			dirtVelocity = glm::vec3(0.0f, -dirtSpeed, 0.0f);
		}

		// Emit dirt particles 
		ParticleProperties particle;
		particle.position = slingBroMotionComponent.position + particleOffset;
		particle.velocity = dirtVelocity;
		particle.velocityVariation = glm::vec3(0.5f, 0.1f, 0.0f);
		particle.colourBegin = glm::vec4(105.0f / 255.0f, 56.0f / 255.0f, 25.0f / 255.0f, 0.0f);
		particle.colourEnd = glm::vec4(105.0f / 255.0f, 56.0f / 255.0f, 25.0f / 255.0f, 1.0f);
		particle.sizeBegin = 4.0f;
		particle.sizeEnd = 1.0f;
		particle.sizeVariation = 1.0f;
		particle.lifeTimeMs = 1000.0f;
		for (int i = 0; i < 2; i++)
			Emit(particle);
		particle.sizeBegin = 8.0f;
		particle.sizeEnd = 2.0f;
		particle.lifeTimeMs = 500.0f;
		for (int i = 0; i < 4; i++)
			Emit(particle);

		// Emit more particles when the bro hits dirt while moving fast
		particle.lifeTimeMs = 1000.0f;
		particle.sizeEnd = 0.1f;
		particle.velocityVariation = glm::vec3(0.3f, 0.2f, 0.05f);
		for (int i = 0; i < int(slingBroSpeed/100.0f); i++)
			Emit(particle);
	}
}

// Collisions between bro and lava tiles - callback function, listening to PhysicsSystem::Collisions
void ParticleSystem::lava_block_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall)
{
	if (!entity_i.IsValid() || !entity_j.IsValid())
		return;

	if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<LavaTile>())
	{
		Motion& slingBroMotionComponent = entity_i.GetComponent<Motion>();
		Motion& lavaMotionComponent = entity_j.GetComponent<Motion>();
		//slingBroMotionComponent.velocity *= 1.1f;

		float slingBroSpeed = glm::length(slingBroMotionComponent.velocity);
		if (slingBroSpeed < 40.0f)
			return;

		// Check orientation of character from lava block and set particle offset correctly
		glm::vec3 dispVec = slingBroMotionComponent.position - lavaMotionComponent.position;
		float angle = atan(dispVec.y, dispVec.x); // angle in radians from lava block to slingbro
		glm::vec3 particleOffset = glm::vec3(0.0f);
		glm::vec3 sparkVelocity = glm::vec3(1.0f);
		float pi = 3.1415926535;
		float offsetMagnitudeX = slingBroMotionComponent.scale.x / 2.0f;
		float offsetMagnitudeY = slingBroMotionComponent.scale.y / 2.0f;
		float sparkSpeed = 0.1f;
		if (angle > -pi / 4 && angle <= pi / 4) // bro to the right of lava block
		{
			particleOffset = glm::vec3(-offsetMagnitudeX, 0.0f, 10.0f);
			sparkVelocity = glm::vec3(sparkSpeed, 0.0f, 0.0f);
		}
		else if (angle > pi / 4 && angle <= 3 * pi / 4) // bro below lava block
		{
			particleOffset = glm::vec3(0.0f, -offsetMagnitudeY, 10.0f);
			sparkVelocity = glm::vec3(0.0f, sparkSpeed, 0.0f);
		}
		else if (angle > 3 * pi / 4 && angle <= 5 * pi / 4) // bro to the left of lava block
		{
			particleOffset = glm::vec3(offsetMagnitudeX, 0.0f, 10.0f);
			sparkVelocity = glm::vec3(-sparkSpeed, 0.0f, 0.0f);
		}
		else // bro above lava block
		{
			particleOffset = glm::vec3(0.0f, offsetMagnitudeY, 10.0f);
			sparkVelocity = glm::vec3(0.0f, -sparkSpeed, 0.0f);
		}

		// Emit spark particles 
		ParticleProperties particle;
		particle.position = slingBroMotionComponent.position + particleOffset;
		particle.velocity = sparkVelocity;
		particle.velocityVariation = glm::vec3(0.5f, 0.1f, 0.0f);
		particle.colourBegin = glm::vec4(1.0f, 0.4f, 0.0f, 1.0f);
		particle.colourEnd = glm::vec4(0.9f, 0.9f, 0.0f, 0.8f);
		particle.sizeBegin = 6.0f;
		particle.sizeEnd = 4.0f;
		particle.sizeVariation = 1.0f;
		particle.lifeTimeMs = 500.0f;
		for (int i = 0; i < 2; i++)
			Emit(particle);
		particle.sizeBegin = 8.0f;
		particle.sizeEnd = 4.0f;
		particle.colourEnd = glm::vec4(1.0f, 0.0f, 0.0f, 0.8f);
		for (int i = 0; i < 3; i++)
			Emit(particle);

		// Emit smoke particles with slight upward velocity
		particle.velocity = glm::vec3(0.0f, -0.025f, -0.01f);
		particle.velocityVariation = glm::vec3(0.1f, 0.05f, 0.0);
		particle.colourBegin = glm::vec4(0.8f, 0.7f, 0.7f, 0.8f);
		particle.colourEnd = glm::vec4(0.1f, 0.1f, 0.1f, 0.0f);
		particle.sizeBegin = 20.0f;
		particle.sizeEnd = 60.0f;
		particle.sizeVariation = 5.0f;
		particle.lifeTimeMs = 5000.0f;
		for (int i = 0; i < 5; i++)
			Emit(particle);

		// Emit lava splash particles 
		particle.velocity = glm::vec3(0.0f, -0.0025f, 0.01f);
		particle.velocityVariation = glm::vec3(0.4f, 0.1f, 0.0);
		particle.colourBegin = glm::vec4(0.8f, 0.0f, 0.0f, 0.2f);
		particle.colourEnd = glm::vec4(0.9f, 0.1f, 0.0f, 1.0f);
		particle.sizeBegin = 1.0f;
		particle.sizeEnd = 10.0f;
		particle.sizeVariation = 10.0f;
		particle.lifeTimeMs = 1000.0f;
		for (int i = 0; i < 1; i++)
			Emit(particle);
	}

}

void ParticleSystem::clearParticles()
{
	for (Particle& particle : m_ParticlePool)
	{
		particle.lifeRemaining = 0.0f;
		particle.active = false;
		particle.currentColour = glm::vec4(0.0f);
	}
}

std::vector<Particle> ParticleSystem::GetActiveParticles() const
{
	std::vector<Particle> activeParticles;
	for (const Particle& particle : m_ParticlePool)
		if (particle.active)
			activeParticles.push_back(particle);
	return activeParticles;
}
