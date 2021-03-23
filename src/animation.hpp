#pragma once

#include "common.hpp"
#include "Entity.h"
#include "Scene.h"
#include "render_components.hpp"
#include "render.hpp"
#include "common.hpp"

struct Animation
{
	std::string cacheResourceKey;
	glm::vec2 baseAnimStartOffset;		// the spritesheet off of the idle / regular animation
	glm::vec2 currentAnimStartOffset;	// the spritesheet offset of the active animation
	int baseNumFrames = 2;			// number of frames for the idle / regular animation
	int currentNumFrames = 2;		// number of frames for the active animation
	int currentFrameNumber = 0;		// the frame number we are currently at in the active animation
	float currentFrameTime = 0.0f;
	float frameLengthMs = 1000.0f;
	bool hasCollisionAnimation = false;
	bool playingCollisionAnimation = false;

	Animation(std::string cacheResourceKey, glm::vec2 animStartOffset, int numAnimationFrames, float frameDisplayTimeMs, bool hasCollisionAnimation)
		: cacheResourceKey(cacheResourceKey), baseAnimStartOffset(animStartOffset), currentAnimStartOffset(baseAnimStartOffset), 
		baseNumFrames(numAnimationFrames), currentNumFrames(baseNumFrames), frameLengthMs(frameDisplayTimeMs), hasCollisionAnimation(hasCollisionAnimation)
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
			glm::vec2 spritesheetOffset(currentAnimStartOffset.x + currentFrameNumber, currentAnimStartOffset.y);
			ShadedMesh& shadedMesh = cache_resource(cacheResourceKey);
			glBindVertexArray(*shadedMesh.mesh.vao.data());
			TexturedVertex vertices[4];
			vertices[0].position = { -1.f / 2, +1.f / 2, 0.f };
			vertices[1].position = { +1.f / 2, +1.f / 2, 0.f };
			vertices[2].position = { +1.f / 2, -1.f / 2, 0.f };
			vertices[3].position = { -1.f / 2, -1.f / 2, 0.f };
			// Usese spritesheet offset to calculate texture coordinates and pass updated vbo to the shader
			vertices[0].texcoord = { (SPRITE_PIXEL_WIDTH * spritesheetOffset.x) / SPRITESHEET_PIXEL_WIDTH, (SPRITE_PIXEL_HEIGHT * spritesheetOffset.y + SPRITE_PIXEL_HEIGHT) / SPRITESHEET_PIXEL_HEIGHT };
			vertices[1].texcoord = { (SPRITE_PIXEL_WIDTH * spritesheetOffset.x + SPRITE_PIXEL_WIDTH) / SPRITESHEET_PIXEL_WIDTH, (SPRITE_PIXEL_HEIGHT * spritesheetOffset.y + SPRITE_PIXEL_HEIGHT) / SPRITESHEET_PIXEL_HEIGHT };
			vertices[2].texcoord = { (SPRITE_PIXEL_WIDTH * spritesheetOffset.x + SPRITE_PIXEL_WIDTH) / SPRITESHEET_PIXEL_WIDTH, (SPRITE_PIXEL_HEIGHT * spritesheetOffset.y) / SPRITESHEET_PIXEL_HEIGHT };
			vertices[3].texcoord = { (SPRITE_PIXEL_WIDTH * spritesheetOffset.x) / SPRITESHEET_PIXEL_WIDTH, (SPRITE_PIXEL_HEIGHT * spritesheetOffset.y) / SPRITESHEET_PIXEL_HEIGHT };
			// Set vertex buffer
			glBindBuffer(GL_ARRAY_BUFFER, shadedMesh.mesh.vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 
			gl_has_errors();
		}
	}

	void playCollisionAnimation(int numFrames, int yOffset)
	{
		if (!hasCollisionAnimation)
			return;

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
