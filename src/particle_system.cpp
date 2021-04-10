#include "particle_system.hpp"
#include "entities/windy_grass.hpp"
#include "entities/lava_tile.hpp"
#include "entities/ground_tile.hpp"
#include "entities/grassy_tile.hpp"
#include "entities/slingbro.hpp"
#include "entities/speed_powerup.hpp"
#include "entities/beehive_enemy.hpp"
#include "render.hpp"
#include "world.hpp"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

// Particle system structure based on example done by youtuber The Cherno in https://www.youtube.com/watch?v=GK0jHlv3e3w

std::mt19937 Random::s_RandomEngine;
std::uniform_int_distribution<std::mt19937::result_type> Random::s_Distribution(1, 100);

ParticleSystem* ParticleSystem::instance = nullptr;

const float WIND_MAGNITUDE = 0.005;

ParticleSystem::ParticleSystem(uint32_t maxNumParticles)
	: m_PoolIndex(maxNumParticles - 1), m_ParticleMesh(nullptr), m_ParticleMeshInstanced(nullptr), m_BeeSwarms(std::vector<BeeSwarm*>())
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

	std::string keyBeeMesh = "particleSystemBeeMesh";
	ShadedMesh* beeMesh = &cache_resource(keyBeeMesh);

	if (beeMesh->effect.program.resource == 0) {
		RenderSystem::createBeeMesh(*beeMesh, "bee_shader");
	}
	
	// Store references to the potentially re-used mesh objects
	m_ParticleMesh = particleMesh;
	m_ParticleMeshInstanced = particleMeshInstanced;
	m_BeeMesh = beeMesh;
}

ParticleSystem* ParticleSystem::GetInstance()
{
	if (!instance)
		instance = new ParticleSystem(MAX_NUM_PARTICLES);
	
	return instance;
}

uint32_t ParticleSystem::NumBeesTargetingEntity(uint32_t entityID)
{
	uint32_t numBeestargetingEntity = 0;
	for (BeeSwarm* swarm : m_BeeSwarms)
		if (swarm->targetedPlayerEntityID == entityID)
			numBeestargetingEntity++;
	return numBeestargetingEntity;
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

		if (particle.affectedByWind)
		{
			// larger particles get blown away slower, smaller particles get blown faster
			particle.velocity.x -= (1 / particle.currentSize) * WIND_MAGNITUDE;
		}
	}

	for (BeeSwarm* swarm : m_BeeSwarms)
	{
		// Update the swarm position to be the player position if the swarm is chasing that player
		if (swarm->isChasing)
			swarm->position = swarm->scene->m_Registry.get<Motion>((entt::entity)swarm->targetedPlayerEntityID).position;
		
		for (Bee& bee : swarm->bees)
		{
			// Handle individual bee movement logic
			updateBeeProperties(bee, swarm->position);
		}
	}
}

void ParticleSystem::updateBeeProperties(Bee& bee, glm::vec3 swarmPosition)
{
	// Recalculate offset from the center of the swarm
	bee.offsetFromSwarmCenter = bee.position - swarmPosition;
	// Update bee velocity in swarm-like fashion
	// If a bee is too far from the center of the swarm, gently start nudging it back
	if (glm::length(bee.offsetFromSwarmCenter) > 100.0f)
		bee.velocity -= bee.offsetFromSwarmCenter / 10000.0f;
	// If a bee is really far from the center of the swarm, send it flying back
	if (glm::length(bee.offsetFromSwarmCenter) > 200.0f)
	{
		bee.velocity -= bee.offsetFromSwarmCenter / 2000.0f;
		bee.velocity = bee.velocity + (glm::vec3((0.5f - Random::Float()), (0.5f - Random::Float()), (0.5f - Random::Float())));
	}
	// Slow down bees that are very close to the center and going very fast
	// This is to stop the hive from oscillating back and forth
	if (glm::length(bee.offsetFromSwarmCenter) < 200.0f && glm::length(bee.velocity) > 4.0f)
		bee.velocity *= 0.95f;
	// Slow down bees that are moving too fast
	if (glm::length(bee.velocity) > 12.0f)
		bee.velocity *= 0.9f;
	// Update bee positions
	bee.position += bee.velocity;
	// Add some randomness to the bee's velocity
	bee.velocity = bee.velocity + (glm::vec3(0.5f - Random::Float(), 0.5f - Random::Float(), 0.5f - Random::Float())) / 5.0f;
	bee.velocity *= 0.995;

	// Update bee rotations depending on movement pattern, with some randomness
	bee.rotationX += bee.velocity.x * (0.5f - Random::Float()) / 2.0f;
	bee.rotationY += bee.velocity.y * (0.5f - Random::Float()) / 2.0f;
	bee.rotationZ += bee.velocity.z * (0.5f - Random::Float()) / 2.0f;
}

void ParticleSystem::Emit(const ParticleProperties& particleProps)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.active = true;
	particle.affectedByWind = particleProps.affectedByWind;
	particle.position = particleProps.position;
	particle.rotation = Random::Float() * 2.0f * PI;

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
		if (WorldSystem::ActiveScene->m_Weather != WeatherTypes::Rain)
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
		float offsetMagnitudeX = slingBroMotionComponent.scale.x / 2.0f;
		float offsetMagnitudeY = slingBroMotionComponent.scale.y / 2.0f;
		if (angle > -PI / 4 && angle <= PI / 4) // bro to the right of block
		{
			particleOffset = glm::vec3(-offsetMagnitudeX, 0.0f, 10.0f);
			dirtVelocity = glm::vec3(dirtSpeed, 0.0f, 0.0f);
		}
		else if (angle > PI / 4 && angle <= 3 * PI / 4) // bro below block
		{
			particleOffset = glm::vec3(0.0f, -offsetMagnitudeY, 10.0f);
			dirtVelocity = glm::vec3(0.0f, dirtSpeed, 0.0f);
		}
		else if (angle > 3 * PI / 4 && angle <= 5 * PI / 4) // bro to the left of block
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

// Collisions between bro and beehives - callback function, listening to PhysicsSystem::Collisions
void ParticleSystem::beehive_collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall)
{
	if (!entity_i.IsValid() || !entity_j.IsValid())
		return;

	if (entity_i.HasComponent<SlingBro>() && entity_j.HasComponent<BeeHiveEnemy>())
	{
		auto& slingBroEntity = entity_i;
		auto& beeHiveEntity = entity_j;

		// Make the bees chase the player who hit their hive
		BeeHiveEnemy& beeHiveComponent = beeHiveEntity.GetComponent<BeeHiveEnemy>();
		BeeSwarm* collidedHivesBeeSwarm = beeHiveComponent.hiveSwarm;
		collidedHivesBeeSwarm->isChasing = true;
		collidedHivesBeeSwarm->targetedPlayerEntityID = slingBroEntity.GetEntityID();

		// Give points for collecting honey from the hive
		Turn& turnComponent = slingBroEntity.GetComponent<Turn>();
		if (!beeHiveComponent.HasBeenHarvestedByPlayer(slingBroEntity.GetEntityID()))
		{
			turnComponent.points += POINTS_GAINED_HARVESTING_HONEY;
			beeHiveComponent.harvestedByPlayers.push_back(slingBroEntity.GetEntityID());

			// Emit some honey gloop particles
			ParticleProperties particle;
			particle.position = beeHiveEntity.GetComponent<Motion>().position;
			particle.velocity = glm::vec3(0.0f, -0.05f, 0.0f);;
			particle.velocityVariation = glm::vec3(0.2f, 0.125f, 0.2f);
			particle.colourBegin = glm::vec4(250.0f / 255.0f, 189.0f / 255.0f, 42.0f / 255.0f, 1.0f);
			particle.colourEnd = glm::vec4(200.0f / 255.0f, 139.0f / 255.0f, 0.0f / 255.0f, 0.2f);
			particle.sizeBegin = 20.0f;
			particle.sizeEnd = 2.0f;
			particle.sizeVariation = 2.0f;
			particle.lifeTimeMs = 4000.0f;
			for (int i = 0; i < 100; i++)
				Emit(particle);
		}
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
		float offsetMagnitudeX = slingBroMotionComponent.scale.x / 2.0f;
		float offsetMagnitudeY = slingBroMotionComponent.scale.y / 2.0f;
		float sparkSpeed = 0.1f;
		if (angle > -PI / 4 && angle <= PI / 4) // bro to the right of lava block
		{
			particleOffset = glm::vec3(-offsetMagnitudeX, 0.0f, 10.0f);
			sparkVelocity = glm::vec3(sparkSpeed, 0.0f, 0.0f);
		}
		else if (angle > PI / 4 && angle <= 3 * PI / 4) // bro below lava block
		{
			particleOffset = glm::vec3(0.0f, -offsetMagnitudeY, 10.0f);
			sparkVelocity = glm::vec3(0.0f, sparkSpeed, 0.0f);
		}
		else if (angle > 3 * PI / 4 && angle <= 5 * PI / 4) // bro to the left of lava block
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

void ParticleSystem::weather_listener(ECS_ENTT::Scene* scene)
{
	ParticleProperties particle;

	if (scene->m_Weather == WeatherTypes::Rain) {
		// Emit rain particles
		particle.position = {Random::Float() * (scene->m_Size.x + 100.f), .0, 0.0};
		particle.velocity = glm::vec3(0.0f, 0.4f, 0.01f);
		particle.velocityVariation = glm::vec3(0.0f, 0.2f, 0.0);
		particle.colourBegin = glm::vec4(0.58f, 0.66f, 0.7f, 1.0f);
		particle.colourEnd = glm::vec4(0.58f, 0.66f, 0.7f, 1.0f);
		particle.sizeBegin = 7.0f;
		particle.sizeEnd = 7.0f;
		particle.sizeVariation = 1.0f;
		particle.lifeTimeMs = scene->m_Size.y * 10.f;
		particle.affectedByWind = true;

		Emit(particle);
	} else if (scene->m_Weather == WeatherTypes::Snow) {
		// Emit snow particles
		particle.position = {Random::Float() * (scene->m_Size.x + 100.f), .0, 0.0};
		particle.velocity = glm::vec3(0.0f, 0.2f, 0.01f);
		particle.velocityVariation = glm::vec3(0.0f, 0.2f, 0.0);
		particle.colourBegin = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		particle.colourEnd = glm::vec4(0.9f, 0.9f, 0.92f, 0.8f);
		particle.sizeBegin = 10.0f;
		particle.sizeEnd = 10.0f;
		particle.sizeVariation = 10.0f;
		particle.lifeTimeMs = scene->m_Size.y * 10.f;
		particle.affectedByWind = true;

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

void ParticleSystem::clearBeeSwarms()
{
	for (BeeSwarm* swarm : m_BeeSwarms)
		swarm->bees.clear();
	m_BeeSwarms.clear();
}

std::vector<Particle> ParticleSystem::GetActiveParticles() const
{
	std::vector<Particle> activeParticles;
	for (const Particle& particle : m_ParticlePool)
		if (particle.active)
			activeParticles.push_back(particle);
	return activeParticles;
}

BeeSwarm* ParticleSystem::CreateBeeSwarm(glm::vec3 swarmCenterPosition, unsigned int numberOfBees)
{
	BeeSwarm* swarm = new BeeSwarm(swarmCenterPosition, numberOfBees);
	m_BeeSwarms.push_back(swarm);
	return swarm;
}

BeeSwarm::BeeSwarm(glm::vec3 swarmCenterPosition, unsigned int numberOfBees)
	: position(swarmCenterPosition), numBees(numberOfBees)
{
	// Initialize all the bees in the swarm with randomized starting positions, velocities, rotations
	for (int i = 0; i < numBees; i++)
	{
		// Bee properties
		glm::vec3 offsetFromSwarmCenter = glm::vec3(100.0f * (0.5f - Random::Float()), 100.0f * (0.5f - Random::Float()), 10.0f * (0.5f - Random::Float()));
		glm::vec3 velocity = glm::vec3(0.5f - Random::Float(), 0.5f - Random::Float(), 0.5f - Random::Float());
		float rotationX = Random::Float() * 2 * 3.14;
		float rotationY = Random::Float() * 2 * 3.14;
		float rotationZ = Random::Float() * 2 * 3.14;
		float size = 8.0f + (8.0f*Random::Float());
		glm::vec4 colour = glm::vec4(1.0f, 1.0f, 0.05f, 1.0f);
		Bee bee = { swarmCenterPosition + offsetFromSwarmCenter, offsetFromSwarmCenter, velocity, rotationX, rotationY, rotationZ, size, colour };
		bees.push_back(bee);
	}
}
