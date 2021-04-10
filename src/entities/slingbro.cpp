#include <text.hpp>
#include "slingbro.hpp"
#include "render.hpp"
#include "animation.hpp"

std::map<BroType, std::string> bro_keys =
		{
				{BroType::ORANGE, "slingbro_orange"},
				{BroType::PINK, "slingbro_pink"}
		};

ECS_ENTT::Entity SlingBro::createOrangeSlingBro(vec3 position, ECS_ENTT::Scene* scene)
{
	return SlingBro::createSlingBro(position, scene, BroType::ORANGE, false);
}

ECS_ENTT::Entity SlingBro::createPinkSlingBro(vec3 position, ECS_ENTT::Scene* scene)
{
	return SlingBro::createSlingBro(position, scene, BroType::PINK, false);
}

ECS_ENTT::Entity SlingBro::createOrangeSlingBroProfile(vec3 position, ECS_ENTT::Scene* scene)
{
	return SlingBro::createSlingBro(position, scene, BroType::ORANGE, true);
}

ECS_ENTT::Entity SlingBro::createPinkSlingBroProfile(vec3 position, ECS_ENTT::Scene* scene)
{
	return SlingBro::createSlingBro(position, scene, BroType::PINK, true);
}

ECS_ENTT::Entity SlingBro::createSlingBro(vec3 position, ECS_ENTT::Scene *scene, BroType type, bool isProfile)
{
	std::string key = bro_keys.at(type);

	if (isProfile)
	{
		key += "profile";
	}
	ShadedMesh& resource = cache_resource(key);

	ECS_ENTT::Entity slingBroEntity = scene->CreateEntity(key);

	vec2 spritesheet_offset = vec2(0, type);

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache).
	slingBroEntity.AddComponent<ShadedMeshRef>(resource);

	// Reset the SlingBro colour when created
	resource.texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	// Setting up SlingBro position.
	Motion& motionComponent = slingBroEntity.AddComponent<Motion>();
	motionComponent.position = position;
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.mesh.original_size.x * SPRITE_SCALE, resource.mesh.original_size.y * SPRITE_SCALE, 1.0f };

	if (isProfile)
	{
		slingBroEntity.AddComponent<Text>();

		TexturedVertex vertices[4];
		std::string path;
		switch (type)
		{
		case BroType::ORANGE:
			path = "bro_basic.png";
			vertices[0].position = { -0.97f, +0.98f, 0.f };
			vertices[1].position = { -0.92f, +0.98f, 0.f };
			vertices[2].position = { -0.92f, +0.90f, 0.f };
			vertices[3].position = { -0.97f, +0.90f, 0.f };
			break;
		case BroType::PINK:
			vertices[0].position = { -0.97f, +0.88f, 0.f };
			vertices[1].position = { -0.92f, +0.88f, 0.f };
			vertices[2].position = { -0.92f, +0.80f, 0.f };
			vertices[3].position = { -0.97f, +0.80f, 0.f };
			path = "kawaii_bro-1.png";
		}
		if (resource.effect.program.resource == 0) {
			RenderSystem::createProfileSprite(resource, textures_path(path), "dialogue_box", vertices);
		}
		motionComponent.scale = { resource.mesh.original_size.x * 100, resource.mesh.original_size.y * 100, 1.0f };

		slingBroEntity.AddComponent<IgnoreSave>();
		slingBroEntity.AddComponent<IgnorePhysics>();
		auto& profile = slingBroEntity.AddComponent<PlayerProfile>();

		switch (type)
		{
			case BroType::ORANGE:
			{
				profile.broType = BroType::ORANGE;
				profile.playerName = "Timmy";
				break;
			}
			case BroType::PINK:
			{
				profile.broType = BroType::PINK;
				profile.playerName = "Tammy";
			}
		}

	}
	else
	{
		if (resource.effect.program.resource == 0) {
			RenderSystem::createSprite(resource, textures_path("bro_characters_spritesheet.png"), "textured", spritesheet_offset);
		}

		// Add component to control sling magnitude
		SlingMotion& slingComponent = slingBroEntity.AddComponent<SlingMotion>();
		slingComponent.isClicked = false;
		slingComponent.direction = { 0.0f, 0.0f };
		slingComponent.magnitude = { 10.f, 20.f };

		// Add countdown for ending turn
		auto& turn = slingBroEntity.AddComponent<Turn>();

		// Set up the animation component
		slingBroEntity.AddComponent<Animation>(key, spritesheet_offset, 6, 200.0f, true);

		// Add gravity
		slingBroEntity.AddComponent<Gravity>();

		// Add mass
		slingBroEntity.AddComponent<Mass>();

		// Add slingbro component
		slingBroEntity.AddComponent<SlingBro>();

		switch (type)
		{
			case BroType::ORANGE:
				slingBroEntity.AddComponent<OrangeBro>();
				break;
			case BroType::PINK:
				slingBroEntity.AddComponent<PinkBro>();
		}
	}

	return slingBroEntity;
}
