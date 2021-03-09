#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"
#include "render_components.hpp"
#include <render.hpp>

struct Animation
{
	std::string cacheResourceKey;
	std::string spritesheetFilepath;
	glm::vec2 baseAnimStartOffset;		// the spritesheet off of the idle / regular animation
	glm::vec2 currentAnimStartOffset;	// the spritesheet offset of the active animation
	int baseNumFrames = 2;			// number of frames for the idle / regular animation
	int currentNumFrames = 2;		// number of frames for the active animation
	int currentFrameNumber = 0;		// the frame number we are currently at in the active animation
	float currentFrameTime = 0.0f;
	float frameLengthMs = 1000.0f;
	bool playingCollisionAnimation = false;

	Animation(std::string cacheResourceKey, std::string spritesheetFilepath, glm::vec2 animStartOffset, int numAnimationFrames, float frameDisplayTimeMs)
		: cacheResourceKey(cacheResourceKey), spritesheetFilepath(spritesheetFilepath), baseAnimStartOffset(animStartOffset), currentAnimStartOffset(baseAnimStartOffset), baseNumFrames(numAnimationFrames), currentNumFrames(baseNumFrames), frameLengthMs(frameDisplayTimeMs)
	{}

	void step(float elapsed_ms)
	{
		currentFrameTime += elapsed_ms;
		if (currentFrameTime >= frameLengthMs)
		{
			// Time to switch to the next animation frame
			currentFrameTime -= frameLengthMs;
			if (currentFrameNumber < currentNumFrames - 1)
				currentFrameNumber++;
			else
			{
				if (playingCollisionAnimation)
				{
					// return to idle / regular animation
					playingCollisionAnimation = false;
					currentAnimStartOffset = baseAnimStartOffset;
					currentFrameNumber = 0;
					currentNumFrames = baseNumFrames;
				}
				currentFrameNumber = 0;
			}

			// Progress to the next frame in the animation
			ShadedMesh& shadedMesh = cache_resource(cacheResourceKey);
			RenderSystem::createSprite(shadedMesh, spritesheetFilepath, "textured", { currentAnimStartOffset.x + currentFrameNumber, currentAnimStartOffset.y });
		}
	}

	void playCollisionAnimation(int numFrames, int yOffset)
	{
		if (playingCollisionAnimation)
		{
			// restart the current animation
			currentFrameNumber = 0;
			return; 
		}

		playingCollisionAnimation = true;
		currentAnimStartOffset = glm::vec2(currentAnimStartOffset.x, currentAnimStartOffset.y + yOffset);
		currentFrameNumber = 0;
		currentNumFrames = numFrames;
	}

};

class AnimationSystem
{
public:

	void step(float elapsed_ms, ECS_ENTT::Scene* activeScene);

	void collision_listener(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, bool hit_wall);

};
