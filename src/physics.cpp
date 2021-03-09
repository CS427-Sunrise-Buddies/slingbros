// internal
#include <entities/speed_powerup.hpp>
#include "physics.hpp"
#include "debug.hpp"
#include "world.hpp"
#include <sstream>
#include <iostream>

// up down left right since rectangle only has 4 sides.
static const vec2 directions[] = {
		vec2(0.0f, 1.0f),
		vec2(0.0f, -1.0f),
		vec2(-1.0f, 0.0f),
		vec2(1.0f, 0.0f),
};

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
	vec2 scene_size = WorldSystem::ActiveScene->m_Size;

	return xpos - radius_i < 0.f || xpos + radius_i > scene_size.x || ypos - radius_i < 0.f ||
		   ypos + radius_i > scene_size.y;
}

void reflect_and_add_friction_to_entity(float& velocity, float friction)
{
	velocity *= -1;
	velocity -= velocity * friction;
}

bool intersects(Motion circle, Motion rect)
{
	vec2 circleDistance;
	circleDistance.x = abs(circle.position.x - rect.position.x);
	circleDistance.y = abs(circle.position.y - rect.position.y);

	if (circleDistance.x > (rect.scale.x / 2 + circle.scale.x))
		return false;
	if (circleDistance.y > (rect.scale.y / 2 + circle.scale.x))
		return false;

	if (circleDistance.x <= (rect.scale.x / 2))
		return true;
	if (circleDistance.y <= (rect.scale.y / 2))
		return true;

	float cornerDistance_sq = (circleDistance.x - rect.scale.x / 2) * (circleDistance.x - rect.scale.x / 2) +
							  (circleDistance.y - rect.scale.y / 2) * (circleDistance.y - rect.scale.y / 2);

	return (cornerDistance_sq <= (circle.scale.x * circle.scale.x));
}

// get the direction of a vector. note: vector is in 2d
VectorDir vector_dir(vec2 v)
{
	int dir_index = 0; // corresponding to VectorDir
	float highest_dot_product = 0.f; // max 1.f, when the angle is 0 degrees

	for (int i = 0; i < 4; i++)
	{
		// dot product to find the angle between the directions vector and the vector in question
		float dot_prod = glm::dot(glm::normalize(v), directions[i]);
		if (dot_prod > highest_dot_product)
		{
			highest_dot_product = dot_prod; // we want the angle to be as small as possible
			dir_index = i;
		}
	}
	return (VectorDir)dir_index;
}

bool is_circle_rect_collision(vec3 circle_center_to_closest_point_on_rect, Motion& circle)
{
	return glm::length(circle_center_to_closest_point_on_rect) < (circle.scale.x / 2);
}

// distance from circle's center to the point on the rect that is closest to the circle
vec3 circle_rect_distance(Motion& circle, Motion& rect, ECS_ENTT::Entity circle_entity)
{
	vec3 half_rect = { rect.scale.x / 2.0f, rect.scale.y / 2.0f, 0 };
	// distance between the circle center and the rect center
	vec3 center_to_center_vector = circle.position - rect.position;
	vec3 clamped_distance = glm::clamp(center_to_center_vector, -half_rect, half_rect);
	vec3 closest_point_to_circle_on_rect = rect.position + clamped_distance;

	return closest_point_to_circle_on_rect - circle.position;
}


void PhysicsSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.

	auto motionEntitiesView = WorldSystem::ActiveScene->m_Registry.view<Motion>(); // position

	// update positions for all entities
	for (auto entityID : motionEntitiesView)
	{
		ECS_ENTT::Entity entity_i = ECS_ENTT::Entity(entityID, WorldSystem::ActiveScene);
		auto& motionComponent_i = entity_i.GetComponent<Motion>();

		if (entity_i.HasComponent<Gravity>())
		{
			auto gravity = entity_i.GetComponent<Gravity>();
			// gravity based on time passed
			motionComponent_i.velocity.y += gravity.gravitational_constant * (elapsed_ms / 1000.0f);
			// horizontal friction
			motionComponent_i.velocity.x -= motionComponent_i.velocity.x * (elapsed_ms / 1000.0f) * 0.3;
		}

		float step_seconds = 1.0f * (elapsed_ms / 1000.f);

		motionComponent_i.position += motionComponent_i.velocity * step_seconds;

	}

	// check for collisions between all entities
	for (auto entityID_i : motionEntitiesView)
	{
		ECS_ENTT::Entity entity_i = ECS_ENTT::Entity(entityID_i, WorldSystem::ActiveScene);
		auto& motionComponent_i = entity_i.GetComponent<Motion>();

		// First check if the entity is colliding with any of the scene bounds
		if (check_wall_collisions(motionComponent_i))
		{
			const float x_pos = motionComponent_i.position.x;
			const float y_pos = motionComponent_i.position.y;
			vec2 bounding_box = { abs(motionComponent_i.scale.x), abs(motionComponent_i.scale.y) };
			float radius_i = sqrt(pow(bounding_box.x / 2.0f, 2.f) + pow(bounding_box.y / 2.0f, 2.f));

			vec2 size = WorldSystem::ActiveScene->m_Size;
			float friction = 0.1;

			if (!entity_i.HasComponent<BouncyTile>())
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

		float radius = motionComponent_i.scale.x / 2;

		// Next check if any two entities are colliding
		WorldSystem::ActiveScene->m_Registry.view<Motion>().each([&](const auto entityID_j, auto &&...)
		{
			ECS_ENTT::Entity entity_j = ECS_ENTT::Entity(entityID_j, WorldSystem::ActiveScene);
			if (entity_j == entity_i)
			{
				return;
			}
			auto& motionComponent_j = entity_j.GetComponent<Motion>();

			//////////////////////////// Collision between a non-tile entity and a tile ////////////////////////////
			// check for Tile BouncyTile and other Object Entity collisions
			// for now, Walls are stationary to simplify things
			if (!entity_i.HasComponent<BouncyTile>() && entity_j.HasComponent<BouncyTile>())
			{
				// entity_i is not a wall, entity_j is a wall.
				// circle to rectangle collisions
				vec3 collision = circle_rect_distance(motionComponent_i, motionComponent_j, entity_i);
				if (is_circle_rect_collision(collision, motionComponent_i))
				{
					VectorDir direction_i = vector_dir(glm::vec2(collision));
					if (direction_i >= VectorDir::LEFT) // left or right - need to move position.x
					{
						// flip direction and multiply some friction
						motionComponent_i.velocity.x = -motionComponent_i.velocity.x * 0.9;
						float move_out_distance = radius - std::abs(collision.x);
						if (direction_i == LEFT)
						{
							motionComponent_i.position.x += move_out_distance;
						}
						else
						{
							motionComponent_i.position.x -= move_out_distance;
						}
					}
					else if (direction_i <= VectorDir::DOWN) // up or down - need to move position.y
					{
						// flip direction and multiply some friction
						motionComponent_i.velocity.y = -motionComponent_i.velocity.y * 0.9;
						float move_out_distance = radius - std::abs(collision.y);
						if (direction_i == UP)
						{
							motionComponent_i.position.y -= move_out_distance;
						}
						else
						{
							motionComponent_i.position.y += move_out_distance;
						}
					}
					runCollisionCallbacks(entity_i, entity_j, true);
				}
			}
			//////////////////////////////////////////////////////////////////////////////

			// Check that the entities aren't the same, aren't both walls, and that they collide
			if (entity_i != entity_j && !entity_i.HasComponent<BouncyTile>() &&
				!entity_j.HasComponent<BouncyTile>() &&
				collides(motionComponent_i, motionComponent_j))
			{
				// Create a collision event - notify observers
				runCollisionCallbacks(entity_i, entity_j, false);
			}
		});

	}

	(void)window_size_in_game_units;

	// Visualization for debugging the position and scale of objects
	if (DebugSystem::in_debug_mode)
	{
		for (auto& entityID : motionEntitiesView)
		{
			ECS_ENTT::Entity entity = ECS_ENTT::Entity(entityID, WorldSystem::ActiveScene);
			auto& m = entity.GetComponent<Motion>();

			if (entity.HasComponent<Tile>()) {
				DebugSystem::createBox(m.position, get_bounding_box(m), m.scale);
			}
			else {
				DebugSystem::createCircle(m.position, m.scale);
			}
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
