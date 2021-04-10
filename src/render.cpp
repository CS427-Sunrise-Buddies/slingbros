// internal
#include "render.hpp"
#include "render_components.hpp"

#include "entities/slingbro.hpp"

#include "world.hpp"
#include "text.hpp"

#include <iostream>

void RenderSystem::drawTexturedMesh(ECS_ENTT::Entity entity, const mat4& view, const mat4& projection)
{
	auto& motion = entity.GetComponent<Motion>();
	auto& texmesh = *entity.GetComponent<ShadedMeshRef>().reference_to_cache;

	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	// Process any deformations 
	if (entity.HasComponent<Deformation>())
	{
		Deformation& deformationComponent = entity.GetComponent<Deformation>();
		transform.rotate(deformationComponent.angleRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		transform.scale(glm::vec3(deformationComponent.scaleX, deformationComponent.scaleY, 0.0f));
		transform.rotate(-deformationComponent.angleRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	transform.rotate(motion.angle, glm::vec3(0.0f, 0.0f, 1.0f));
	transform.scale(motion.scale);

	// Setting shaders
	glUseProgram(texmesh.effect.program);
	glBindVertexArray(texmesh.mesh.vao);
	gl_has_errors();

	// Enabling alpha channel and depth test for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	gl_has_errors();

	GLuint time_uloc = glGetUniformLocation(texmesh.effect.program, "time");
	GLint transform_uloc = glGetUniformLocation(texmesh.effect.program, "transform");
	GLint view_uloc = glGetUniformLocation(texmesh.effect.program, "view"); // allows for camera movement
	GLint projection_uloc = glGetUniformLocation(texmesh.effect.program, "projection");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, texmesh.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texmesh.mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(texmesh.effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(texmesh.effect.program, "in_texcoord");
	GLint in_color_loc = glGetAttribLocation(texmesh.effect.program, "in_color");
	if (in_texcoord_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(sizeof(vec3))); // note the stride to skip the preceeding vertex position
		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texmesh.texture.texture_id);
	}
	else if (in_color_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(sizeof(vec3)));

		if (entity.HasComponent<LightUp>())
		{
			GLint light_up_uloc = glGetUniformLocation(texmesh.effect.program, "light_up");
			auto& lightUpComponent = entity.GetComponent<LightUp>();
			glUniform1i(light_up_uloc, (GLint)lightUpComponent.isLit);
		}
		else
		{
			GLint light_up_uloc = glGetUniformLocation(texmesh.effect.program, "light_up");
			glUniform1i(light_up_uloc, (GLint)0);
		}
	}
	else
	{
		throw std::runtime_error("This type of entity is not yet supported");
	}
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(texmesh.effect.program, "fcolor");
	glUniform4fv(color_uloc, 1, (float*)&texmesh.texture.color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);
	//GLsizei num_triangles = num_indices / 3;

	// Setting uniform values to the currently bound program
	glUniform1f(time_uloc, static_cast<float>(glfwGetTime() * 10.0f));
	glUniformMatrix4fv(transform_uloc, 1, GL_FALSE, (float*)&transform.matrix);
	glUniformMatrix4fv(view_uloc, 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
}

void RenderSystem::drawParticle(Particle particle, ShadedMesh* particleMesh, const mat4& view, const mat4& projection)
{
	Transform transform;
	transform.translate(particle.position);
	transform.rotate(particle.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	transform.scale(glm::vec3(particle.currentSize, particle.currentSize, 1.0f));

	// Setting shaders
	glUseProgram(particleMesh->effect.program);
	glBindVertexArray(particleMesh->mesh.vao);
	gl_has_errors();

	// Enabling alpha channel and depth test for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	gl_has_errors();

	GLint transform_uloc = glGetUniformLocation(particleMesh->effect.program, "transform");
	GLint view_uloc = glGetUniformLocation(particleMesh->effect.program, "view"); 
	GLint projection_uloc = glGetUniformLocation(particleMesh->effect.program, "projection");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, particleMesh->mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleMesh->mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(particleMesh->effect.program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(0));
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(particleMesh->effect.program, "fcolor");
	glUniform4fv(color_uloc, 1, (float*)&particle.currentColour);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);

	// Setting uniform values to the currently bound program
	glUniformMatrix4fv(transform_uloc, 1, GL_FALSE, (float*)&transform.matrix);
	glUniformMatrix4fv(view_uloc, 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
}

// Draw the intermediate texture to the screen, with some distortion to simulate water
void RenderSystem::drawToScreen()
{
	// Setting shaders
	glUseProgram(screen_sprite.effect.program);
	glBindVertexArray(screen_sprite.mesh.vao);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(&window, &w, &h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	
	// Disable alpha channel for mapping the screen texture onto the real screen
	glDisable(GL_BLEND); // we have a single texture without transparency. Areas with alpha <1 cab arise around the texture transparency boundary, enabling blending would make them visible.
	glDisable(GL_DEPTH_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, screen_sprite.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screen_sprite.mesh.ibo); // Note, GL_ELEMENT_ARRAY_BUFFER associates indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();

	// Draw the screen texture on the quad geometry
	gl_has_errors();

	// Set clock
	GLuint time_uloc       = glGetUniformLocation(screen_sprite.effect.program, "time");
	glUniform1f(time_uloc, static_cast<float>(glfwGetTime() * 10.0f));
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in the same VBO)
	GLint in_position_loc = glGetAttribLocation(screen_sprite.effect.program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	GLint in_texcoord_loc = glGetAttribLocation(screen_sprite.effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3)); // note the stride to skip the preceeding vertex position
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screen_sprite.texture.texture_id);

	// Draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr); // two triangles = 6 vertices; nullptr indicates that there is no offset from the bound index buffer
	glBindVertexArray(0);
	gl_has_errors();
}

void RenderSystem::drawParticlesInstanced(ParticleSystem* particleSystem, const mat4& view, const mat4& projection)
{
	int index = 0;
	for (const Particle& particle : particleSystem->GetActiveParticles())
	{
		Transform transform;
		// Set transform instance
		transform.translate(particle.position);
		transform.rotate(particle.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		transform.scale(glm::vec3(particle.currentSize, particle.currentSize, 1.0f));
		transformMatrices[index] = transform.matrix;
		// Set colour instance
		particleColours[index] = particle.currentColour;
		index++;
	}

	// Set all non-active instanced particles to be invisible
	for (int i = index; i < MAX_NUM_PARTICLES; i++)
		particleColours[i] = glm::vec4(0.0f);

	auto particleMesh = particleSystem->GetParticleMeshInstanced();

	// Bind shaders and VAO
	glUseProgram(particleMesh->effect.program);
	glBindVertexArray(particleMesh->mesh.vao);
	gl_has_errors();

	// Enabling alpha channel and depth test for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	gl_has_errors();

	// Instanced colour VBO
	glBindBuffer(GL_ARRAY_BUFFER, instanced_colours_VBO);
	size_t vec4_size = sizeof(glm::vec4);
	glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_NUM_PARTICLES * vec4_size, &particleColours[0]);

	// Instanced transform VBO
	glBindBuffer(GL_ARRAY_BUFFER, instanced_transforms_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_NUM_PARTICLES * 4 * sizeof(vec4_size), &transformMatrices[0]);

	GLint view_uloc = glGetUniformLocation(particleMesh->effect.program, "view");
	GLint projection_uloc = glGetUniformLocation(particleMesh->effect.program, "projection");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, particleMesh->mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleMesh->mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(particleMesh->effect.program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(0));
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);

	// Setting uniform values to the currently bound program
	glUniformMatrix4fv(view_uloc, 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0, MAX_NUM_PARTICLES);
	glBindVertexArray(0);
}

void RenderSystem::drawBee(Bee bee, ShadedMesh* beeMesh, const mat4& view, const mat4& projection)
{
	Transform transform;
	transform.translate(bee.position);
	transform.rotate(bee.rotationX, glm::vec3(1.0f, 0.0f, 0.0f));
	transform.rotate(bee.rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
	transform.rotate(bee.rotationZ, glm::vec3(0.0f, 0.0f, 1.0f));
	transform.scale(glm::vec3(bee.size, bee.size, 1.0f));

	// Setting shaders
	glUseProgram(beeMesh->effect.program);
	glBindVertexArray(beeMesh->mesh.vao);
	gl_has_errors();

	// Enabling alpha channel and depth test for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	gl_has_errors();

	GLint transform_uloc = glGetUniformLocation(beeMesh->effect.program, "transform");
	GLint view_uloc = glGetUniformLocation(beeMesh->effect.program, "view");
	GLint projection_uloc = glGetUniformLocation(beeMesh->effect.program, "projection");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, beeMesh->mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, beeMesh->mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(beeMesh->effect.program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(0));
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(beeMesh->effect.program, "fcolor");
	glUniform4fv(color_uloc, 1, (float*)&bee.colour);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);

	// Setting uniform values to the currently bound program
	glUniformMatrix4fv(transform_uloc, 1, GL_FALSE, (float*)&transform.matrix);
	glUniformMatrix4fv(view_uloc, 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(vec2 window_size_in_game_units, Camera& activeCamera, ParticleSystem* particleSystem)
{
	// Getting size of window
	ivec2 frame_buffer_size; // in pixels
	glfwGetFramebufferSize(&window, &frame_buffer_size.x, &frame_buffer_size.y);

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// Clearing backbuffer
	glViewport(0, 0, frame_buffer_size.x, frame_buffer_size.y);
	glDepthRange(0.00001, 10);
	glClearColor(176.f/255.f, 208.f/255.f, 211.f/255.f, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();

	// Get the view and projection matrices from the active camera to be passed to the vertex shaders
	glm::mat4 viewMatrix = activeCamera.GetViewMatrix();
	glm::mat4 projMatrix = activeCamera.GetProjectionMatrix(); 

	// Draw all textured meshes that have a position and size component
	ECS_ENTT::Scene* scene = WorldSystem::ActiveScene;
	for (auto entityID : scene->m_Registry.view<ShadedMeshRef>())
	{
		ECS_ENTT::Entity entity{ entityID, scene };
		if (!entity.HasComponent<Motion>())
			continue;
		// Note, its not very efficient to access elements indirectly via the entity albeit iterating through all Sprites in sequence
		drawTexturedMesh(entity, viewMatrix, projMatrix);
		gl_has_errors();
	}

	// Draw all particles:
	// Using instancing
	drawParticlesInstanced(particleSystem, viewMatrix, projMatrix);
	gl_has_errors();
	// Using one draw call per particle
	/*for (const Particle& particle : particleSystem->GetParticles())
	{
		if (!particle.active)
			continue;
		drawParticle(particle, particleSystem->GetParticleMesh(), viewMatrix, projMatrix);
		gl_has_errors();
	}*/

	// Draw all bees
	// Using one draw call per bee
	for (const BeeSwarm* beeSwarm : particleSystem->GetBeeSwarms())
		for (const Bee& bee : beeSwarm->bees)
		{
			drawBee(bee, particleSystem->GetBeeMesh(), viewMatrix, projMatrix);
			gl_has_errors();
		}

	// Draw text components to the screen
	// NOTE: for simplicity, text components are drawn in a second pass,
	// on top of all texture mesh components. This should be reasonable
	// for nearly all use cases. If you need text to appear behind meshes,
	// consider using a depth buffer during rendering and adding a
	// Z-component or depth index to all rendererable components.
	for (auto entityID : scene->m_Registry.view<Text>()) {
		const Text& text = scene->m_Registry.get<Text>(entityID);
		drawText(text, window_size_in_game_units);
	}
	
	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(&window);
}

void gl_has_errors()
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR)
		return;
	
	const char* error_str = "";
	while (error != GL_NO_ERROR)
	{
		switch (error)
		{
		case GL_INVALID_OPERATION:
			error_str = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error_str = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error_str = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error_str = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error_str = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		std::cerr << "OpenGL:" << error_str << std::endl;
		error = glGetError();
	}
	throw std::runtime_error("last OpenGL error:" + std::string(error_str));
}
