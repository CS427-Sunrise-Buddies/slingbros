#pragma once

#include "common.hpp"
//#include "tiny_ecs.hpp"
#include "render_components.hpp"

#include "Scene.h"
#include "Entity.h"
#include "Camera.h"
#include "particle_system.hpp"

struct InstancedMesh;
struct ShadedMesh;

// OpenGL utilities
void gl_has_errors();

// System responsible for setting up OpenGL and for rendering all the 
// visual entities in the game
class RenderSystem
{
public:
	// Initialize the window
	RenderSystem(GLFWwindow& window);

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw(vec2 window_size_in_game_units, Camera& activeCamera, ParticleSystem* particleSystem);

	// Expose the creating of visual representations to other systems
	static void createSprite(ShadedMesh& mesh_container, std::string texture_path, std::string shader_name, glm::vec2 spritesheetOffset = vec2(-1.0f, -1.0f));
	static void createProfileSprite(ShadedMesh& mesh_container, std::string texture_path, std::string shader_name, TexturedVertex (&vertices)[4]);
	static void createDialogueSprite(ShadedMesh& mesh_container, std::string texture_path, std::string shader_name);
	static void createBackgroundSprite(ShadedMesh& mesh_container, std::string texture_path, std::string shader_name);
	static void createParticle(ShadedMesh& particleMesh, std::string shader_name);
	static void createBeeMesh(ShadedMesh& beeMesh, std::string shader_name);
	static void createColoredMesh(ShadedMesh& mesh_container, std::string shader_name);

private:
	// Initialize the screeen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water shader
	void initScreenTexture();

	// Internal drawing functions for each entity type
	void drawTexturedMesh(ECS_ENTT::Entity entity, const mat4& view, const mat4& projection);
	void drawParticle(Particle particle, ShadedMesh* particleMesh, const mat4& view, const mat4& projection);
	void drawParticlesInstanced(ParticleSystem* particleSystem, const mat4& view, const mat4& projection);
	void drawBee(Bee bee, ShadedMesh* beeMesh, const mat4& view, const mat4& projection);
	void drawToScreen();

	// Window handle
	GLFWwindow& window;

	// Screen texture handles
	GLuint frame_buffer;
	ShadedMesh screen_sprite;
	GLResource<RENDER_BUFFER> depth_render_buffer_id;
	ECS_ENTT::Entity screen_state_entity;

	// Instanced arrays for both transform matrices and colour 
	glm::mat4* transformMatrices;
	glm::vec4* particleColours; 

	// Keep track of instancing vertex buffers
	unsigned int instanced_colours_VBO;
	unsigned int instanced_transforms_VBO;
};
