// internal
#include "render.hpp"
#include "render_components.hpp"

#include "world.hpp"

#include <iostream>
#include <fstream>

// World initialization
RenderSystem::RenderSystem(GLFWwindow& window) :
	window(window), transformMatrices(new glm::mat4[MAX_NUM_PARTICLES]), particleColours(new glm::vec4[MAX_NUM_PARTICLES]), instanced_colours_VBO(0), instanced_transforms_VBO(0)
{
	glfwMakeContextCurrent(&window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	gl3w_init();

	// Create a frame buffer
	frame_buffer = 0;
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	initScreenTexture();

	// Set starting transformMatrices and particleColours for instanced rendering
	glm::mat4 identity = glm::mat4(1.0f);
	glm::vec4 defaultColour = glm::vec4(1.0f);
	for (int i = 0; i < MAX_NUM_PARTICLES; i++)
	{
		transformMatrices[i] = identity;
		particleColours[i] = defaultColour;
	}

	// Setup Particle System VAO
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	auto particleMesh = ParticleSystem::GetInstance()->GetParticleMeshInstanced();
	// Setting shaders
	glUseProgram(particleMesh->effect.program);
	glBindVertexArray(particleMesh->mesh.vao);
	gl_has_errors();
	// Instanced colour VBO
	glGenBuffers(1, &instanced_colours_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanced_colours_VBO);
	size_t vec4_size = sizeof(glm::vec4);
	glBufferData(GL_ARRAY_BUFFER, MAX_NUM_PARTICLES * vec4_size, &particleColours[0], GL_DYNAMIC_DRAW);
	// Setup for vec4 instanceColour
	GLint instance_colour_loc = glGetAttribLocation(particleMesh->effect.program, "instanceColour");
	glEnableVertexAttribArray(instance_colour_loc);
	glVertexAttribPointer(instance_colour_loc, 4, GL_FLOAT, GL_FALSE, vec4_size, (void*)0);
	// Tell OpenGL to increment to next colour vec4 every render call
	glVertexAttribDivisor(instance_colour_loc, 1);
	// Instanced transform VBO
	//unsigned int instanced_transforms_VBO;
	glGenBuffers(1, &instanced_transforms_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanced_transforms_VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_NUM_PARTICLES * 4 * sizeof(vec4_size), &transformMatrices[0], GL_DYNAMIC_DRAW);
	// Setup for mat4 transformMatrices (which are just 4 vec4s each)
	GLint instance_transform_loc = glGetAttribLocation(particleMesh->effect.program, "instanceTransform");
	glEnableVertexAttribArray(instance_transform_loc);
	glVertexAttribPointer(instance_transform_loc, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);
	glEnableVertexAttribArray(instance_transform_loc + 1);
	glVertexAttribPointer(instance_transform_loc + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(1 * vec4_size));
	glEnableVertexAttribArray(instance_transform_loc + 2);
	glVertexAttribPointer(instance_transform_loc + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(2 * vec4_size));
	glEnableVertexAttribArray(instance_transform_loc + 3);
	glVertexAttribPointer(instance_transform_loc + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(3 * vec4_size));
	// Tell OpenGL to increment to next matrix every render call
	glVertexAttribDivisor(instance_transform_loc, 1);
	glVertexAttribDivisor(instance_transform_loc + 1, 1);
	glVertexAttribDivisor(instance_transform_loc + 2, 1);
	glVertexAttribDivisor(instance_transform_loc + 3, 1);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

RenderSystem::~RenderSystem()
{
	// delete allocated resources
	glDeleteFramebuffers(1, &frame_buffer);

	// TODO: update the following to work with EnTT ECS
	// remove all entities created by the render system
	/*while (ECS::registry<Motion>.entities.size() > 0)
		ECS::ContainerInterface::remove_all_components_of(ECS::registry<Motion>.entities.back());
	while (ECS::registry<ShadedMeshRef>.entities.size() > 0)
		ECS::ContainerInterface::remove_all_components_of(ECS::registry<ShadedMeshRef>.entities.back());*/
}

// Create a new sprite and register it with ECS
void RenderSystem::createSprite(ShadedMesh& sprite, std::string texture_path, std::string shader_name, glm::vec2 spritesheetOffset)
{
	if (texture_path.length() > 0)
		sprite.texture.load_from_file(texture_path.c_str());

	// The position corresponds to the center of the texture.
	TexturedVertex vertices[4];
	vertices[0].position = { -1.f/2, +1.f/2, 0.f };
	vertices[1].position = { +1.f/2, +1.f/2, 0.f };
	vertices[2].position = { +1.f/2, -1.f/2, 0.f };
	vertices[3].position = { -1.f/2, -1.f/2, 0.f };

	// Check if the spritesheet offset has been changed from default values
	if (spritesheetOffset == glm::vec2(-1.0f, -1.0f))
	{
		vertices[0].texcoord = { 0.f, 1.f };
		vertices[1].texcoord = { 1.f, 1.f };
		vertices[2].texcoord = { 1.f, 0.f };
		vertices[3].texcoord = { 0.f, 0.f };
	}
	else
	{
		// Spritesheet offset has been defined -- use that to calculate texture coordinates that should be passed to the shader
		vertices[0].texcoord = { (SPRITE_PIXEL_WIDTH * spritesheetOffset.x) / SPRITESHEET_PIXEL_WIDTH, (SPRITE_PIXEL_HEIGHT * spritesheetOffset.y + SPRITE_PIXEL_HEIGHT) / SPRITESHEET_PIXEL_HEIGHT };
		vertices[1].texcoord = { (SPRITE_PIXEL_WIDTH * spritesheetOffset.x + SPRITE_PIXEL_WIDTH) / SPRITESHEET_PIXEL_WIDTH, (SPRITE_PIXEL_HEIGHT * spritesheetOffset.y + SPRITE_PIXEL_HEIGHT) / SPRITESHEET_PIXEL_HEIGHT };
		vertices[2].texcoord = { (SPRITE_PIXEL_WIDTH * spritesheetOffset.x + SPRITE_PIXEL_WIDTH) / SPRITESHEET_PIXEL_WIDTH, (SPRITE_PIXEL_HEIGHT * spritesheetOffset.y) / SPRITESHEET_PIXEL_HEIGHT };
		vertices[3].texcoord = { (SPRITE_PIXEL_WIDTH * spritesheetOffset.x) / SPRITESHEET_PIXEL_WIDTH, (SPRITE_PIXEL_HEIGHT * spritesheetOffset.y) / SPRITESHEET_PIXEL_HEIGHT };
	}

	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	glGenVertexArrays(1, sprite.mesh.vao.data());
	glGenBuffers(1, sprite.mesh.vbo.data());
	glGenBuffers(1, sprite.mesh.ibo.data());
	gl_has_errors();

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, sprite.mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // sizeof(TexturedVertex) * 4
	gl_has_errors();

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite.mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // sizeof(uint16_t) * 6
	gl_has_errors();

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	// Loading shaders
	sprite.effect.load_from_file(shader_path(shader_name) + ".vs.glsl", shader_path(shader_name) + ".fs.glsl");
}

void RenderSystem::createProfileSprite(ShadedMesh& sprite, std::string texture_path, std::string shader_name, TexturedVertex (&vertices)[4])
{
	if (texture_path.length() > 0)
		sprite.texture.load_from_file(texture_path.c_str());

	vertices[0].texcoord = { 0.f, 0.f };
	vertices[1].texcoord = { 1.f, 0.f };
	vertices[2].texcoord = { 1.f, 1.f };
	vertices[3].texcoord = { 0.f, 1.f };


// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	glGenVertexArrays(1, sprite.mesh.vao.data());
	glGenBuffers(1, sprite.mesh.vbo.data());
	glGenBuffers(1, sprite.mesh.ibo.data());
	gl_has_errors();

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, sprite.mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // sizeof(TexturedVertex) * 4
	gl_has_errors();

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite.mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // sizeof(uint16_t) * 6
	gl_has_errors();

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	// Loading shaders
	sprite.effect.load_from_file(shader_path(shader_name) + ".vs.glsl", shader_path(shader_name) + ".fs.glsl");
}


// Create a new sprite and register it with ECS
void RenderSystem::createDialogueSprite(ShadedMesh& sprite, std::string texture_path, std::string shader_name)
{
	if (texture_path.length() > 0)
		sprite.texture.load_from_file(texture_path.c_str());

	TexturedVertex vertices[4];
	vertices[0].position = { -0.9f, -0.45f, 0.f };
	vertices[1].position = { +0.9f, -0.45f, 0.f };
	vertices[2].position = { +0.9f, -0.9f, 0.f };
	vertices[3].position = { -0.9f, -0.9f, 0.f };

	vertices[0].texcoord = { 0.f, 0.f };
	vertices[1].texcoord = { 1.f, 0.f };
	vertices[2].texcoord = { 1.f, 1.f };
	vertices[3].texcoord = { 0.f, 1.f };


	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	glGenVertexArrays(1, sprite.mesh.vao.data());
	glGenBuffers(1, sprite.mesh.vbo.data());
	glGenBuffers(1, sprite.mesh.ibo.data());
	gl_has_errors();

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, sprite.mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // sizeof(TexturedVertex) * 4
	gl_has_errors();

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite.mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // sizeof(uint16_t) * 6
	gl_has_errors();

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	// Loading shaders
	sprite.effect.load_from_file(shader_path(shader_name) + ".vs.glsl", shader_path(shader_name) + ".fs.glsl");
}

void RenderSystem::createBackgroundSprite(ShadedMesh& sprite, std::string texture_path, std::string shader_name)
{
	if (texture_path.length() > 0)
		sprite.texture.load_from_file(texture_path.c_str());

	// The position corresponds to the center of the texture.
	TexturedVertex vertices[4];
	vertices[0].position = { -1.f / 2, +1.f / 2, 0.f };
	vertices[1].position = { +1.f / 2, +1.f / 2, 0.f };
	vertices[2].position = { +1.f / 2, -1.f / 2, 0.f };
	vertices[3].position = { -1.f / 2, -1.f / 2, 0.f };

	// Set background sprites to be tiled 10 times
	vertices[0].texcoord = { 0.f, 10.f };
	vertices[1].texcoord = { 10.f, 10.f };
	vertices[2].texcoord = { 10.f, 0.f };
	vertices[3].texcoord = { 0.f, 0.f };

	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	glGenVertexArrays(1, sprite.mesh.vao.data());
	glGenBuffers(1, sprite.mesh.vbo.data());
	glGenBuffers(1, sprite.mesh.ibo.data());
	gl_has_errors();

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, sprite.mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // sizeof(TexturedVertex) * 4
	gl_has_errors();

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite.mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // sizeof(uint16_t) * 6
	gl_has_errors();

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	// Loading shaders
	sprite.effect.load_from_file(shader_path(shader_name) + ".vs.glsl", shader_path(shader_name) + ".fs.glsl");
}

void RenderSystem::createParticle(ShadedMesh& particleMesh, std::string shader_name)
{
	// The position corresponds to the center of the texture.
	TexturedVertex vertices[4];
	vertices[0].position = { -1.f / 2, +1.f / 2, 0.f };
	vertices[1].position = { +1.f / 2, +1.f / 2, 0.f };
	vertices[2].position = { +1.f / 2, -1.f / 2, 0.f };
	vertices[3].position = { -1.f / 2, -1.f / 2, 0.f };

	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	glGenVertexArrays(1, particleMesh.mesh.vao.data());
	glGenBuffers(1, particleMesh.mesh.vbo.data());
	glGenBuffers(1, particleMesh.mesh.ibo.data());
	gl_has_errors();

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, particleMesh.mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // sizeof(TexturedVertex) * 4
	gl_has_errors();

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleMesh.mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // sizeof(uint16_t) * 6
	gl_has_errors();

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	// Loading shaders
	particleMesh.effect.load_from_file(shader_path(shader_name) + ".vs.glsl", shader_path(shader_name) + ".fs.glsl");
}

void RenderSystem::createBeeMesh(ShadedMesh& beeMesh, std::string shader_name)
{
	// The position corresponds to the center of the texture.
	TexturedVertex vertices[4];
	vertices[0].position = { -0.8f, +0.4f, 0.0f };
	vertices[1].position = { +0.8f, +0.4f, 0.0f };
	vertices[2].position = { +0.8f, -0.4f, 0.0f };
	vertices[3].position = { -0.8f, -0.4f, 0.0f };

	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	glGenVertexArrays(1, beeMesh.mesh.vao.data());
	glGenBuffers(1, beeMesh.mesh.vbo.data());
	glGenBuffers(1, beeMesh.mesh.ibo.data());
	gl_has_errors();

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, beeMesh.mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 
	gl_has_errors();

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, beeMesh.mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 
	gl_has_errors();

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	// Loading shaders
	beeMesh.effect.load_from_file(shader_path(shader_name) + ".vs.glsl", shader_path(shader_name) + ".fs.glsl");
}

// Load a new mesh from disc and register it with ECS
void RenderSystem::createColoredMesh(ShadedMesh& texmesh, std::string shader_name)
{
	// Vertex Array
	glGenVertexArrays(1, texmesh.mesh.vao.data());
	glGenBuffers(1, texmesh.mesh.vbo.data());
	glGenBuffers(1, texmesh.mesh.ibo.data());
	glBindVertexArray(texmesh.mesh.vao);

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, texmesh.mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ColoredVertex) * texmesh.mesh.vertices.size(), texmesh.mesh.vertices.data(), GL_STATIC_DRAW);

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texmesh.mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * texmesh.mesh.vertex_indices.size(), texmesh.mesh.vertex_indices.data(), GL_STATIC_DRAW);
	gl_has_errors();

	// Note, one could set vertex attributes here...
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	// glEnableVertexAttribArray(0);
	// glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	// Loading shaders
	texmesh.effect.load_from_file(shader_path(shader_name)+".vs.glsl", shader_path(shader_name)+".fs.glsl");
}

// Initialize the screen texture from a standard sprite
void RenderSystem::initScreenTexture()
{
	// Create a sprite withour loading a texture
	createSprite(screen_sprite, "", "water");

	// Initialize the screen texture and its state
	screen_sprite.texture.create_from_screen(&window, depth_render_buffer_id.data());
}
