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


bool aabb_collision(Motion& motion_i, Motion& motion_j)
{
	vec2 dOne = { abs(motion_i.scale.x) / 2, abs(motion_i.scale.y) / 2 };
	vec2 dTwo = { abs(motion_j.scale.x) / 2, abs(motion_j.scale.y) / 2 };
	bool collisionX = motion_i.position.x + dOne.x >= motion_j.position.x - dTwo.x &&
					  motion_j.position.x + dTwo.x >= motion_i.position.x - dOne.x;
	bool collisionY = motion_i.position.y + dOne.y >= motion_j.position.y - dTwo.y &&
					  motion_j.position.y + dTwo.y >= motion_i.position.y - dOne.y;
	return collisionX && collisionY;
}


MaxMin findMinMaxFromMesh(ECS_ENTT::Entity entity, vec2 window_size_in_game_units, bool isDebug)
{
	auto motion = entity.GetComponent<Motion>();
	auto shadedMesh = entity.GetComponent<ShadedMeshRef>();
	MaxMin boundingCoords{};
	boundingCoords.xMax = 0;
	boundingCoords.xMin = window_size_in_game_units.x;
	boundingCoords.yMax = 0;
	boundingCoords.yMin = window_size_in_game_units.y;

	for (ColoredVertex v : shadedMesh.reference_to_cache->mesh.vertices)
	{
		Transform transform;
		transform.translate(motion.position);
		transform.rotate(motion.angle, glm::vec3(0.0f, 0.0f, 1.0f));
		transform.scale(motion.scale);
		vec4 pos = { v.position.x, v.position.y, v.position.z, 1 };
//		v.position.z = 1.f;
		vec4 transformed_v = transform.matrix * vec4(v.position, 1);
		if (transformed_v.x > boundingCoords.xMax)
		{
			boundingCoords.xMax = transformed_v.x;
		}
		if (transformed_v.x < boundingCoords.xMin)
		{
			boundingCoords.xMin = transformed_v.x;
		}
		if (transformed_v.y > boundingCoords.yMax)
		{
			boundingCoords.yMax = transformed_v.y;
		}
		if (transformed_v.y < boundingCoords.yMin)
		{
			boundingCoords.yMin = transformed_v.y;
		}
//		if (isDebug)
//		{
//			vec2 scale_horizontal_line = {5, 5};
//			DebugSystem::createLine({transformed_v.x, transformed_v.y}, scale_horizontal_line,
//					0); // big dot
//		}
	}
	if (boundingCoords.xMax == 0 &&
		boundingCoords.xMin == window_size_in_game_units.x &&
		boundingCoords.yMax == 0 &&
		boundingCoords.yMin == window_size_in_game_units.y) // no mesh, use motion.scale only
	{
		vec2 dy = { 0, abs(motion.scale.y) / 2 };
		vec2 dx = { abs(motion.scale.x) / 2, 0 };

		boundingCoords.yMax = motion.position.y + dy.y;
		boundingCoords.yMin = motion.position.y - dy.y;
		boundingCoords.xMax = motion.position.x + dx.x;
		boundingCoords.xMin = motion.position.x - dx.x;
	}
	return boundingCoords;
}


bool collides(ECS_ENTT::Entity entity_i, ECS_ENTT::Entity entity_j, vec2 window_size_in_game_units, bool isDebug)
{

	auto motion_i = entity_i.GetComponent<Motion>();
	auto motion_j = entity_j.GetComponent<Motion>();


	if (aabb_collision(motion_i, motion_j))
	{
		// check exact vertices
		MaxMin boundingCoords_i = findMinMaxFromMesh(entity_i, window_size_in_game_units, isDebug);
		MaxMin boundingCoords_j = findMinMaxFromMesh(entity_j, window_size_in_game_units, isDebug);
		bool x_axis_collision = boundingCoords_i.xMax >= boundingCoords_j.xMin &&
								boundingCoords_j.xMax >= boundingCoords_i.xMin;
		bool y_axis_collision = boundingCoords_i.yMax >= boundingCoords_j.yMin &&
								boundingCoords_j.yMax >= boundingCoords_i.yMin;
		return x_axis_collision && y_axis_collision;

	}
	return false;
//	auto dp = motion1.position - motion2.position;
//	float dist_squared = dot(dp, dp);
//	float other_r = std::sqrt(
//			std::pow(get_bounding_box(motion1).x / 2.0f, 2.f) + std::pow(get_bounding_box(motion1).y / 2.0f, 2.f));
//	float my_r = std::sqrt(
//			std::pow(get_bounding_box(motion2).x / 2.0f, 2.f) + std::pow(get_bounding_box(motion2).y / 2.0f, 2.f));
//	float r = max(other_r, my_r);
//	if (dist_squared < r * r)
//		return true;
//	return false;
}

bool check_wall_collisions(Motion m)
{
	const float xpos = m.position.x;
	const float ypos = m.position.y;
	vec2 bounding_box = { abs(m.scale.x), abs(m.scale.y) };
	float radius_i = sqrt(pow(bounding_box.x / 2.0f, 2.f)
						  + pow(bounding_box.y / 2.0f, 2.f));
	vec2 scene_size = WorldSystem::GameScene->m_Size;

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
	{ return false; }
	if (circleDistance.y > (rect.scale.y / 2 + circle.scale.x))
	{ return false; }

	if (circleDistance.x <= (rect.scale.x / 2))
	{ return true; }
	if (circleDistance.y <= (rect.scale.y / 2))
	{ return true; }

	float cornerDistance_sq = (circleDistance.x - rect.scale.x / 2) * (circleDistance.x - rect.scale.x / 2) +
							  (circleDistance.y - rect.scale.y / 2) * (circleDistance.y - rect.scale.y / 2);

	return (cornerDistance_sq <= (circle.scale.x * circle.scale.x));
}

// get the direction of a vector. note: vector is in 2d
VectorDir vector_dir(vec2 v)
{
	// up down left right since square only has 4 sides.
	vec2 directions[] = {
			vec2(0.0f, 1.0f),
			vec2(0.0f, -1.0f),
			vec2(-1.0f, 0.0f),
			vec2(1.0f, 0.0f),
	};
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

bool is_circle_rect_collision(vec3 circle_center_to_closest_point_on_square, Motion& circle)
{
	return glm::length(circle_center_to_closest_point_on_square) < (circle.scale.x / 2);
}

vec3 circle_rect_distance(Motion& circle, Motion& square, ECS_ENTT::Entity circle_entity)
{
	vec3 square_half = { square.scale.x / 2.0f, square.scale.y / 2.0f, 0 };
	// distance between the circle center and the square center
	vec3 center_to_center_vector = circle.position - square.position;
	vec3 clamped_distance = glm::clamp(center_to_center_vector, -square_half, square_half);
	vec3 closest_point_to_circle_on_square = square.position + clamped_distance;

	return closest_point_to_circle_on_square - circle.position;

//		if (circle_entity.HasComponent<Projectile>())
//		{
//			std::cout << "circle.position.x " << circle.position.x << std::endl;
//			std::cout << "circle.position.y " << circle.position.y << std::endl;
//
//			std::cout << "circle.scale.x " << circle.scale.x << std::endl;
//			std::cout << "circle.scale.y " << circle.scale.y << std::endl;
//			std::cout << "square.scale.x " << square.scale.x << std::endl;
//			std::cout << "square.scale.y " << square.scale.y << std::endl;
//
//			std::cout << "square.position.x " << square.position.x << std::endl;
//			std::cout << "square.position.y " << square.position.y << std::endl;
//		}
}


void PhysicsSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.

	auto motionEntitiesView = WorldSystem::GameScene->m_Registry.view<Motion>(); // position

	// update positions for all entities
	for (auto entityID : motionEntitiesView)
	{
		ECS_ENTT::Entity entity_i = ECS_ENTT::Entity(entityID, WorldSystem::GameScene);
		auto& motionComponent_i = entity_i.GetComponent<Motion>();

		if (entity_i.HasComponent<Projectile>())
		{
//			std::cout << "Enemy's position.x " << motionComponent_i.scale.x << std::endl;
//			std::cout << "Enemy's position.y " << motionComponent_i.scale.y << std::endl;
//			std::cout << "Enemy's position.z " << motionComponent_i.scale.z << std::endl;

//			std::cout << "j position.x " << motionComponent_j.position.x << std::endl;
//			std::cout << "j position.y " << motionComponent_j.position.y << std::endl;
//				std::cout << "j velocity.x " << motionComponent_j.velocity.x << std::endl;
//				std::cout << "j velocity.y " << motionComponent_j.velocity.y << std::endl;
		}

		if (entity_i.HasComponent<Gravity>() && DebugSystem::is_gravity_on)
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
		ECS_ENTT::Entity entity_i = ECS_ENTT::Entity(entityID_i, WorldSystem::GameScene);
		auto& motionComponent_i = entity_i.GetComponent<Motion>();

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
		if (DebugSystem::in_debug_mode)
		{
//			auto shadedMesh = entity_i.GetComponent<ShadedMeshRef>();
//
//			for (ColoredVertex v : shadedMesh.reference_to_cache->mesh.vertices)
//			{
//				Transform transform;
//				transform.translate(motionComponent_i.position);
//				transform.rotate(motionComponent_i.angle, glm::vec3(0.0f, 0.0f, 1.0f));
//				transform.scale(motionComponent_i.scale);
//				v.position.z = 1.f;
//				vec3 transformed_v = mat3(transform.matrix) * v.position;
//				std::cout << "transformed.x " << transformed_v.x << std::endl;
//				std::cout << "transformed.y " << transformed_v.y << std::endl;
//				std::cout << "transformed.z " << transformed_v.z << std::endl;
//
//				vec3 scale_horizontal_line = { 5, 5, 1 };
			DebugSystem::createLine({ 200, 400, 1 }, { 5, 5, 1 },
					WorldSystem::GameScene); // big dot
//			}
		}

		// Next check if any two entities are colliding
		for (auto entityID_j : motionEntitiesView)
		{
			ECS_ENTT::Entity entity_j = ECS_ENTT::Entity(entityID_j, WorldSystem::GameScene);
			if (entity_j == entity_i)
			{
				continue;
			}
			auto& motionComponent_j = entity_j.GetComponent<Motion>();

			//////////////////////////// Collision between x entity and Walls ////////////////////////////
			// check for Wall Entity and other Object Entity collisions
			// for now, Walls are stationary to simplify things
			if (!entity_i.HasComponent<Wall>() && entity_j.HasComponent<Wall>())
			{
				// entity_i is not a wall, entity_j is a wall.
				// circle to square collisions
				// todo: account for when the bro is at rest - no need to do anything below:
				vec3 collision = circle_rect_distance(motionComponent_i, motionComponent_j, entity_i);
				if (is_circle_rect_collision(collision, motionComponent_i))
				{
					float radius = motionComponent_i.scale.x / 2;
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
				}
			}
			//////////////////////////////////////////////////////////////////////////////

//			if (entity_i.HasComponent<BasicEnemy>())
//			{
//				std::cout << "entity_i = Basic Enemy's position.x " << motionComponent_i.position.x << std::endl;
//			}
//			if (entity_j.HasComponent<BasicEnemy>())
//			{
//				std::cout << "entity_j = Basic Enemy's position.x " << motionComponent_i.position.x << std::endl;
//			}
//			if (entity_i.HasComponent<SlingBro>())
//			{
//				std::cout << "entity_i = SlingBro's position.x " << motionComponent_i.position.x << std::endl;
//			}
//			if (entity_j.HasComponent<SlingBro>())
//			{
//				std::cout << "entity_j = SlingBro's position.x " << motionComponent_i.position.x << std::endl;
//			}
//			if (entity_i.HasComponent<Projectile>())
//			{
//				std::cout << "entity_i = Projectile's position.x " << motionComponent_i.position.x << std::endl;
//			}
//			if (entity_j.HasComponent<Projectile>())
//			{
//				std::cout << "entity_j = Projectile's position.x " << motionComponent_i.position.x << std::endl;
//			}
			// Check that the entities aren't the same, aren't both walls, and that they collide
			if (entity_i != entity_j && !entity_i.HasComponent<Wall>() &&
				!entity_j.HasComponent<Wall>() &&
				collides(entity_i, entity_j, window_size_in_game_units, DebugSystem::in_debug_mode))
			{

				if (entity_i.HasComponent<BasicEnemy>() || entity_j.HasComponent<BasicEnemy>())
				{
					std::cout << "Enemy's position.x " << motionComponent_i.position.x << std::endl;
//					std::cout << "Enemy's position.y " << motionComponent_i.position.y << std::endl;
//					std::cout << "Enemy's position.z " << motionComponent_i.position.z << std::endl;

//			std::cout << "j position.x " << motionComponent_j.position.x << std::endl;
//			std::cout << "j position.y " << motionComponent_j.position.y << std::endl;
//				std::cout << "j velocity.x " << motionComponent_j.velocity.x << std::endl;
//				std::cout << "j velocity.y " << motionComponent_j.velocity.y << std::endl;
				}
//				vec2 pob absolute_i = { std::abs(diff_i.x), std::abs(diff_i.y) };

//				velocity2D_i = velocity2D_i -
//											 ((velocity2D_i - velocity2D_j) *
//											  (position2D_i - position2D_j)) /
//											 (absolute_i * absolute_i) *
//											 (position2D_i - position2D_j);
//				motionComponent_i.velocity = { velocity2D_i.x, velocity2D_i.y, 0 };
//
//				vec2 diff_j = position2D_j - position2D_i;
//				vec2 absolute_j = { std::abs(diff_j.x), std::abs(diff_j.y) };
//
//				velocity2D_j = velocity2D_j -
//											 ((velocity2D_j - velocity2D_i) *
//											  (position2D_j - position2D_i)) /
//											 (absolute_j * absolute_j) *
//											 (position2D_j - position2D_i);
//				motionComponent_j.velocity = { velocity2D_j.x, velocity2D_j.y, 0 };

//				motionComponent_i.velocity.z = 0;
//				motionComponent_j.velocity.z = 0;
				// Create a collision event - notify observers
				runCollisionCallbacks(entity_i, entity_j, false);
			}
		}
	}


	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	(void)window_size_in_game_units;

	// Visualization for debugging the position and scale of objects
//	if (DebugSystem::in_debug_mode)
//	{
//		for (auto& motion : motionEntitiesView)
//		{
//			auto& m = motionEntitiesView.get<Motion>(motion);
//
//			// draw a cross at the position of all objects
//			auto scale_horizontal_line = m.scale;
//			scale_horizontal_line.y *= 0.1f;
//			auto scale_vertical_line = m.scale;
//			scale_vertical_line.x *= 0.1f;
//			DebugSystem::createLine(m.position, scale_horizontal_line, WorldSystem::GameScene);
//			DebugSystem::createLine(m.position, scale_vertical_line, WorldSystem::GameScene);
//		}
//	}
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
