#define _USE_MATH_DEFINES

#include <cmath>

#include <entities/goal_tile.hpp>
#include <iostream>
#include "pathfinding.hpp"
#include "world.hpp"
#include <loader/level_manager.hpp>

MoveToGoal::MoveToGoal() : m_GoalReached(false)
{

}

void MoveToGoal::init(ECS_ENTT::Entity e)
{
	if (m_GoalReached) {
		return;
	}
	// Perform BFS to find the shortest path to goal position

	int rows = WorldSystem::GameScene->m_Map.size();
	int cols = WorldSystem::GameScene->m_Map[0].size();

	// 2d matrix to store whether or not we visited a node already
	bool* visited = new bool[rows * cols];
	// init to all false
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			visited[i * cols + j] = false;

	// first node in BFS will be this AI entity's position
	vec2 root = e.GetComponent<Motion>().position;
	std::queue<vec2> root_path;
	root_path.emplace(root);

	visited[(int) (root.y / SPRITE_SCALE) * cols + (int) (root.x / SPRITE_SCALE)] = true;

	std::queue<std::queue<vec2>> pathQueue;
	pathQueue.emplace(root_path);

	while (!pathQueue.empty()) {
		std::queue<vec2> test_path = pathQueue.front();
		// last item in the path is the most recent node position we've added
		vec2 nodePosition = test_path.back();
		vec2 nodePositionIndices = {nodePosition.x / SPRITE_SCALE, nodePosition.y / SPRITE_SCALE};
		pathQueue.pop();

		std::string test_e = WorldSystem::GameScene->m_Map[(int) nodePositionIndices.y][(int) nodePositionIndices.x];

		if (isGoalTile(test_e)) {
			m_Path = test_path;
			m_GoalReached = true;
			delete[] visited;
			return;
		}
		// if we run into a tile or if there is no ground around this space, stop exploring this path
		if (isTile(test_e) || isFloating(nodePositionIndices.y, nodePositionIndices.x)) {
			continue;
		}

		for (vec2 direction : BFS_DIRECTIONS) {
			vec2 new_node = {nodePosition.x + (direction.x * SPRITE_SCALE),
							 nodePosition.y + (direction.y * SPRITE_SCALE)};
			vec2 new_node_indices = nodePositionIndices + direction;

			if (outOfMapRange(new_node_indices.y, new_node_indices.x)) {
				continue;
			}

			// if we have not explored this path yet, mark as explored and add to queue
			if (!visited[(int) new_node_indices.y * cols + (int) new_node_indices.x]) {
				visited[(int) new_node_indices.y * cols + (int) new_node_indices.x] = true;
				std::queue<vec2> new_path = test_path;
				new_path.emplace(new_node);
				pathQueue.emplace(new_path);
			}
		}
	}
	delete[] visited;
}

BehaviorTree::State MoveToGoal::process(ECS_ENTT::Entity e, float elapsed_ms)
{
	if (m_Path.empty()) {
		return BehaviorTree::State::Successful;
	}

	// Get the source and destination positions
	auto& motion = e.GetComponent<Motion>();
	auto position = motion.position;
	auto source = vec2(position);      // Initial x,y position of the AI
	auto destination = m_Path.front(); // x,y position of the next tile to move towards

	// Already at destination
	if (source.x == destination.x && source.y == destination.y) {
		// Finished moving to target tile so remove from path
		m_Path.pop();
		destination = m_Path.front();

		// Set the next intermediate target destination
		auto& ai = e.GetComponent<AI>();
		ai.target = destination;
	}

	// Compute the direction of travel
	auto direction = normalize(destination - source);

	// Rotate the AI
	motion.angle = M_PI_2 * direction.y;

	// Travel in the intended direction
	motion.velocity = vec3(direction * AI_SPEED, motion.velocity.z);

	return BehaviorTree::State::Running;
}

bool MoveToGoal::isGoalTile(std::string s)
{
	return s == T1;
}

bool MoveToGoal::isTile(std::string s)
{
	return s == T2 || s == T3 || s == T4 || s == T6 || s == T7 || s == H0 || s == H1;
}

bool MoveToGoal::isFloating(int y, int x)
{
	for (vec2 direction : BFS_DIRECTIONS) {
		int test_y = y + (int) direction.y;
		int test_x = x + (int) direction.x;

		if (outOfMapRange(test_y, test_x)) {
			continue;
		}

		std::string test_e = WorldSystem::GameScene->m_Map[test_y][test_x];
		// if there is a ground based tile around this position, the snail has a way to traverse to it
		if (isTile(test_e)) {
			return false;
		}
	}
	return true;
}

bool MoveToGoal::outOfMapRange(int y, int x)
{
	int rows = WorldSystem::GameScene->m_Map.size();
	int cols = WorldSystem::GameScene->m_Map[0].size();

	return y >= rows || x >= cols || y < 0 || x < 0;
}
