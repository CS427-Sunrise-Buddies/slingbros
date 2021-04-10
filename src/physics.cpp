// internal
#include "entities/speed_powerup.hpp"
#include "physics.hpp"
#include "debug.hpp"
#include "world.hpp"
#include <iostream>
#include <entities/slingbro.hpp>

static const float FRICTION = 0.1f;

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
	return dist_squared < r * r;
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

void reflect_and_add_friction_to_entity(float& velocity)
{
	velocity *= -1;
	velocity -= velocity * FRICTION;
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


vec2 get_clamped_distance(Motion& circle, Motion& rect){
	// use vec2 because some entities could have z != 0.
	vec2 half_rect = { rect.scale.x / 2.0f, rect.scale.y / 2.0f };
	// distance between the circle center and the rect center
	vec2 center_to_center_vector = circle.position - rect.position;
	return glm::clamp(center_to_center_vector, -half_rect, half_rect);
}

// get the direction of a vector. note: vector is in 2d
VectorDir vector_dir(vec2 v, vec2 clamped, Motion& motionComponent)
{
	int dir_index = 0; // corresponding to VectorDir
	float highest_dot_product = 0.f; // max 1.f, when the angle is 0 degrees
	float dot_prod;
	for (int i = 0; i < 4; i++)
	{
		// dot product to find the angle between the directions vector and the vector in question
		dot_prod = glm::dot(glm::normalize(v), directions[i]);
		if (dot_prod > highest_dot_product)
		{
			highest_dot_product = dot_prod; // we want the angle to be as small as possible
			dir_index = i;
		}
	}

	// To fix infinite bouncing boi. clamped.y < 0 means that it is on top of a tile
	if (clamped.x == clamped.y && clamped.y < 0 && dir_index == VectorDir::UP && abs(motionComponent.velocity.x) < 5.0f)
	{
		dir_index = clamped.x < 0 ? VectorDir::RIGHT : VectorDir::LEFT;
	}

	return (VectorDir)dir_index;
}

bool is_circle_rect_collision(vec2 circle_center_to_closest_point_on_rect, Motion& circle)
{
	return glm::length(circle_center_to_closest_point_on_rect) < (circle.scale.x / 2);
}

// distance from circle's center to the point on the rect that is closest to the circle
vec2 circle_rect_distance(Motion& circle, Motion& rect, vec2 clamped)
{
	vec2 closest_point_to_circle_on_rect = glm::vec2(rect.position) + clamped;

	vec2 circle_center_to_closest_point_on_rect = closest_point_to_circle_on_rect - glm::vec2(circle.position);
	return circle_center_to_closest_point_on_rect;
}


void PhysicsSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	(void)window_size_in_game_units;

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
			auto friction = WorldSystem::ActiveScene->m_Weather == WeatherTypes::Rain
							? RAIN_HORIZONTAL_FRICTION_MAGNITUDE
							: HORIZONTAL_FRICTION_MAGNITUDE;
			motionComponent_i.velocity.x -= motionComponent_i.velocity.x * (elapsed_ms / 1000.0f) * friction;
		}

		float step_seconds = 1.0f * (elapsed_ms / 1000.f);

		// Move pathfinding AI
		if (entity_i.HasComponent<AI>())
		{
			// Pathfinding AI have target node
			auto ai = entity_i.GetComponent<AI>();
			if (ai.target)
			{
				// Move AI min distance between expected step and target node destination
				auto current_position = motionComponent_i.position;
				auto next_position = current_position + motionComponent_i.velocity * step_seconds;
				auto target = ai.target.value();
				if (distance(current_position, next_position) > distance(vec2(current_position), target))
				{
					motionComponent_i.position = vec3(target, motionComponent_i.position.z);
					motionComponent_i.can_move = false;
				}
			}
		}

		if (motionComponent_i.can_move)
		{
			motionComponent_i.position += motionComponent_i.velocity * step_seconds;
		}
	}

	// check for collisions between all entities
	for (auto entityID_i : motionEntitiesView)
	{
		ECS_ENTT::Entity entity_i = ECS_ENTT::Entity(entityID_i, WorldSystem::ActiveScene);

		// Disregard physics for all entities with the IgnorePhysics component (all tiles and purely visual entities)
		if (entity_i.HasComponent<IgnorePhysics>()) 
			continue;
		
		auto& motionComponent_i = entity_i.GetComponent<Motion>();

		// First check if the entity is colliding with any of the scene bounds
		if (check_wall_collisions(motionComponent_i))
		{
			const float x_pos = motionComponent_i.position.x;
			const float y_pos = motionComponent_i.position.y;
			vec2 bounding_box = { abs(motionComponent_i.scale.x), abs(motionComponent_i.scale.y) };
			float radius_i = sqrt(pow(bounding_box.x / 2.0f, 2.f) + pow(bounding_box.y / 2.0f, 2.f));

			vec2 size = WorldSystem::ActiveScene->m_Size;

			if (!entity_i.HasComponent<BouncyTile>())
			{
				if (x_pos - radius_i < 0.f) // left wall
				{
					motionComponent_i.position.x = 0.f + radius_i;
					reflect_and_add_friction_to_entity(motionComponent_i.velocity.x);
				}
				else if (x_pos + radius_i > size.x) // right wall
				{
					motionComponent_i.position.x = size.x - radius_i;
					reflect_and_add_friction_to_entity(motionComponent_i.velocity.x);
				}
				else if (y_pos - radius_i < 0.f) // ceiling
				{
					motionComponent_i.position.y = 0.f + radius_i;
					reflect_and_add_friction_to_entity(motionComponent_i.velocity.y);
				}
				else if (y_pos + radius_i > size.y) // floor
				{
					motionComponent_i.position.y = size.y - radius_i;
					reflect_and_add_friction_to_entity(motionComponent_i.velocity.y);
				}
				runCollisionCallbacks(entity_i, entity_i, true);

			}
		}

		float radius = motionComponent_i.scale.x / 2;

		// Next check if any two entities are colliding
		WorldSystem::ActiveScene->m_Registry.view<Motion>().each([&](const auto entityID_j, auto &&...)
		{
			ECS_ENTT::Entity entity_j = ECS_ENTT::Entity(entityID_j, WorldSystem::ActiveScene);

			if (entity_j == entity_i) // Don't need to check if the same entity is colliding with each other
			{
				return;
			}

			auto& motionComponent_j = entity_j.GetComponent<Motion>();

			//////////////////////////// Collision between a non-tile entity and a tile ////////////////////////////
			// check for Tile BouncyTile and other Object Entity collisions
			// for now, Tiles are stationary to simplify things
			if (entity_j.HasComponent<BouncyTile>())
			{
				// entity_i is not a tile, entity_j is a tile.
				// circle to rectangle collisions
				vec2 clamped = get_clamped_distance(motionComponent_i, motionComponent_j);
				vec2 collision = circle_rect_distance(motionComponent_i, motionComponent_j, clamped);
				if (is_circle_rect_collision(collision, motionComponent_i))
				{
					VectorDir direction_i = vector_dir(glm::vec2(collision), clamped, motionComponent_i);
					if (direction_i >= VectorDir::LEFT) // left or right - need to move position.x
					{
						// flip direction and multiply some friction
						motionComponent_i.velocity.x *= VELOCITY_BOUNCE_MULTIPLIER;
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
						motionComponent_i.velocity.y *= VELOCITY_BOUNCE_MULTIPLIER;
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

			// Check that the entities aren't both walls, and that they collide
			if (!entity_j.HasComponent<BouncyTile>() && collides(motionComponent_i, motionComponent_j))
			{
				// Create a collision event - notify observers
				runCollisionCallbacks(entity_i, entity_j, false);
			}
		});
	}

	// Handle bro-bro collision
	auto slingbro_view = WorldSystem::ActiveScene->m_Registry.view<SlingBro>();
	for (auto id_1 : slingbro_view)
	{
		// Retrieve the relevant components of the first bro
		ECS_ENTT::Entity entity_1 = ECS_ENTT::Entity(id_1, WorldSystem::ActiveScene);
		auto& motion_1 = entity_1.GetComponent<Motion>();
		auto& mass_1 = entity_1.GetComponent<Mass>().value;

		// Calculate bro radius
		auto radius_1 = abs(motion_1.scale.x / 2.f);

		// Get x, y positions and velocities of the first bro
		auto x_1 = motion_1.position.x;
		auto y_1 = motion_1.position.y;

		// Handle bro-bro collision
		for (auto id_2 : slingbro_view)
		{
			// Skip collision check with same entity
			if (id_1 == id_2) continue;

			// Retrieve the relevant components of the bros
			ECS_ENTT::Entity entity_2 = ECS_ENTT::Entity(id_2, WorldSystem::ActiveScene);
			auto& motion_2 = entity_2.GetComponent<Motion>();
			auto& mass_2 = entity_2.GetComponent<Mass>().value;

			// Calculate bro radius
			auto radius_2 = abs(motion_2.scale.x / 2.f);

			// Sum radii to get boundary between bro centers
			auto collision_distance = radius_1 + radius_2; // Expected collision distance

			// Calculate actual distance between the two bros
			auto actual_distance = distance(vec2(motion_1.position), vec2(motion_2.position)); // Actual collision distance

			// Bros collide
			if (actual_distance < collision_distance)
			{
				// Get x, y positions and velocities of the second bro
				auto x_2 = motion_2.position.x;
				auto y_2 = motion_2.position.y;

				// Compute x, y points of collision
				auto collision_pt_x = (x_1 * radius_2 + x_2 * radius_1) / collision_distance;
				auto collision_pt_y = (y_1 * radius_2 + y_2 * radius_1) / collision_distance;
				auto collision_pt = vec2(collision_pt_x, collision_pt_y);

				// Compute new velocities of the bros after elastic collision
				auto mass_t = mass_1 + mass_2; // Total mass
				glm::vec2 vel_1 = glm::vec2(motion_1.velocity);
				glm::vec2 vel_2 = glm::vec2(motion_2.velocity); 
				glm::vec2 pos_1 = glm::vec2(motion_1.position);
				glm::vec2 pos_2 = glm::vec2(motion_2.position);
				glm::vec2 new_vel_1 = vel_1 - ((2 * mass_2 / mass_t) * glm::dot(vel_1 - vel_2, pos_1 - pos_2) / glm::length(pos_1 - pos_2) * (pos_1 - pos_2)) / 100.0f;
				glm::vec2 new_vel_2 = vel_2 - ((2 * mass_1 / mass_t) * glm::dot(vel_2 - vel_1, pos_2 - pos_1) / glm::length(pos_2 - pos_1) * (pos_2 - pos_1)) / 100.0f;

				// Calculate overlapping distance
				auto overlap = abs(collision_distance - actual_distance) + 5.f; // Smol epsilon

				// Compute the direction to move each object out
				auto direction_1 = vec2(motion_1.position) - collision_pt; // Direction to move bro 1 out
				auto direction_2 = vec2(motion_2.position) - collision_pt; // Direction to move bro 2 out

				// Amount to move out each entity depends on mass ratio
				// so lighter entities move out more so it visually makes sense
				auto ratio_1 = mass_2 / mass_t; // Equivalent to 1 - m1/mt
				auto ratio_2 = mass_1 / mass_t; // Equivalent to 1 - m2/mt
				motion_1.position += vec3(overlap * ratio_1 * normalize(direction_1), 0.f);
				motion_2.position += vec3(overlap * ratio_2 * normalize(direction_2), 0.f);

				// Update bro velocity components
				motion_1.velocity = { new_vel_1.x, new_vel_1.y, 0.f };
				motion_2.velocity = { new_vel_2.x, new_vel_2.y, 0.f };

				// Deform the characters
				glm::vec2 dispVec = vec2(motion_1.position) - vec2(motion_2.position);
				float angle = atan(dispVec.y, dispVec.x); // angle in radians from one slingbro to the other
				if (entity_1.HasComponent<Deformation>())
					entity_1.RemoveComponent<Deformation>();
				if (entity_2.HasComponent<Deformation>())
					entity_2.RemoveComponent<Deformation>();
				float squish_magnitude_1 = 0.5f + glm::length(new_vel_1) / MAX_VELOCITY;
				float squish_magnitude_2 = 0.5f + glm::length(new_vel_2) / MAX_VELOCITY;
				entity_1.AddComponent<Deformation>(1.0f - (0.2f * squish_magnitude_1), 1.0f + (0.2f * squish_magnitude_1), angle, 100.0f);
				entity_2.AddComponent<Deformation>(1.0f - (0.2f * squish_magnitude_2), 1.0f + (0.2f * squish_magnitude_2), angle, 100.0f);

				// Create a collision event - notify observers
				runCollisionCallbacks(entity_1, entity_2, false);
			}
		}
	}

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
