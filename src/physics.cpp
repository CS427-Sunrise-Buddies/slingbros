// internal
#include <iostream>
#include "physics.hpp"
#include "debug.hpp"
#include "world.hpp"
#include "render_components.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity 
vec2 get_bounding_box(const Motion& motion)
{
	// fabs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You don't
// need to try to use this technique.
bool collides(const Motion& motion1, const Motion& motion2)
{
	auto dp = motion1.position - motion2.position;
	float dist_squared = dot(dp, dp);
	float other_r = std::sqrt(std::pow(get_bounding_box(motion1).x / 2.0f, 2.f) + std::pow(get_bounding_box(motion1).y / 2.0f, 2.f));
	float my_r = std::sqrt(std::pow(get_bounding_box(motion2).x / 2.0f, 2.f) + std::pow(get_bounding_box(motion2).y / 2.0f, 2.f));
	float r = max(other_r, my_r);
	if (dist_squared < r * r)
		return true;
	return false;
}

bool check_wall_collisions(Motion m)
{
	const float xpos = m.position.x;
	const float ypos = m.position.y;
	vec2 bounding_box = { abs(m.scale.x), abs(m.scale.y) };
	float radius_i = sqrt(pow(bounding_box.x / 2.0f, 2.f)
						  + pow(bounding_box.y / 2.0f, 2.f));
	vec2 scene_size = WorldSystem::GameScene->m_Size;

	return xpos - radius_i < 0.f || xpos + radius_i > scene_size.x || ypos - radius_i < 0.f || ypos + radius_i > scene_size.y;
}

void reflect_and_add_friction_to_entity(float& velocity, float friction)
{
	velocity *= -1;
	velocity -= velocity * friction;
}

void PhysicsSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.

	auto motionEntitiesView = WorldSystem::GameScene->m_Registry.view<Motion>(); // position

	// Check for collisions between all moving entities
	for (auto entityID : motionEntitiesView)
	{
		ECS_ENTT::Entity entity_i = ECS_ENTT::Entity(entityID, WorldSystem::GameScene);
		auto& motionComponent_i = entity_i.GetComponent<Motion>();

		if (entity_i.HasComponent<Gravity>()) {
			auto gravity = entity_i.GetComponent<Gravity>();
			motionComponent_i.velocity.y += gravity.gravitational_constant * (elapsed_ms / 1000.0f); // gravity based on time passed
		}

		float step_seconds = 1.0f * (elapsed_ms / 1000.f);

		motionComponent_i.position += motionComponent_i.velocity * step_seconds;

		// First check if the entity is colliding with any of the scene bounds
		if (check_wall_collisions(motionComponent_i))
		{
			const float x_pos = motionComponent_i.position.x;
			const float y_pos = motionComponent_i.position.y;
			vec2 bounding_box = { abs(motionComponent_i.scale.x), abs(motionComponent_i.scale.y) };
			float radius_i = sqrt(pow(bounding_box.x / 2.0f, 2.f) + pow(bounding_box.y / 2.0f, 2.f));

			vec2 size = WorldSystem::GameScene->m_Size;
			float friction = 0.1;

			if (!entity_i.HasComponent<Wall>())
			{
				if (x_pos - radius_i < 0.f) // left wall
				{
					motionComponent_i.position.x = 0.f + radius_i;
					reflect_and_add_friction_to_entity(motionComponent_i.velocity.x, friction);
				}
				else if (x_pos + radius_i > size.x) // right wall
				{
					motionComponent_i.position.x = size.x - radius_i;
					reflect_and_add_friction_to_entity(motionComponent_i.velocity.x, friction);
				}
				else if (y_pos - radius_i < 0.f) // ceiling
				{
					motionComponent_i.position.y = 0.f + radius_i;
					reflect_and_add_friction_to_entity(motionComponent_i.velocity.y, friction);
				}
				else if (y_pos + radius_i > size.y) // floor
				{
					motionComponent_i.position.y = size.y - radius_i;
					reflect_and_add_friction_to_entity(motionComponent_i.velocity.y, friction);
				}
				runCollisionCallbacks(entity_i, entity_i, true);

			}
		}

		// Next check if any two entities are colliding
		for (auto entityID : motionEntitiesView)
		{
			ECS_ENTT::Entity entity_j = ECS_ENTT::Entity(entityID, WorldSystem::GameScene);
			Motion& motionComponent_j = entity_j.GetComponent<Motion>();

			// Check that the entities aren't the same, aren't both walls, and that they collide
			if (entity_i != entity_j && !entity_i.HasComponent<Wall>() &&
										!entity_j.HasComponent<Wall>() &&
										collides(motionComponent_i, motionComponent_j))
			{
				// Create a collision event - notify observers
				runCollisionCallbacks(entity_i, entity_j, false);
			}
		}
	}


	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	(void)window_size_in_game_units;

	// Visualization for debugging the position and scale of objects
	if (DebugSystem::in_debug_mode)
	{
		for (auto& motion : motionEntitiesView)
		{
			auto& m = motionEntitiesView.get<Motion>(motion);

			// draw a cross at the position of all objects
			auto scale_horizontal_line = m.scale;
			scale_horizontal_line.y *= 0.1f;
			auto scale_vertical_line = m.scale;
			scale_vertical_line.x *= 0.1f;
			DebugSystem::createLine(m.position, scale_horizontal_line);
			DebugSystem::createLine(m.position, scale_vertical_line);
		}
	}
}

PhysicsSystem::Collision::Collision(ECS_ENTT::Entity& other)
{
	this->other = other;
}

void PhysicsSystem::attach(std::function<void(ECS_ENTT::Entity, ECS_ENTT::Entity, bool)> fn)
{
	callbacks.push_back(fn);
}

// this should be called after collision detected
void PhysicsSystem::runCollisionCallbacks(ECS_ENTT::Entity i, ECS_ENTT::Entity j, bool hit_wall)
{
	for (std::function<void(ECS_ENTT::Entity, ECS_ENTT::Entity, bool)> fn : callbacks)
	{
		// run the callback functions
		fn(i, j, hit_wall);
	}
}

// todo: example, remove if not needed
void PhysicsSystem::getState()
{
}
