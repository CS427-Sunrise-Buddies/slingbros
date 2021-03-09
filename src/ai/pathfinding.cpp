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
	m_IterationNum = 0;
	// Perform BFS to find the shortest path to goal position

	int rows = WorldSystem::GameScene->m_Map.size();
	int cols = WorldSystem::GameScene->m_Map[0].size();

	// 2d matrix to store whether or not we visited a node already
	bool* visited = new bool[rows * cols];
	// init to all false
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			visited[i*cols+j] = false;

	// first node in BFS will be this AI entity's position
	vec2 root = e.GetComponent<Motion>().position;
	std::queue<vec2> root_path;
	root_path.emplace(root);

	visited[(int)(root.y / SPRITE_SCALE) * cols + (int)(root.x / SPRITE_SCALE)] = true;

	std::queue<std::queue<vec2>> pathQueue;
	pathQueue.emplace(root_path);

	while (!pathQueue.empty()) {
		std::queue<vec2> test_path = pathQueue.front();
		// last item in the path is the most recent node position we've added
		vec2 nodePosition = test_path.back();
		vec2 nodePositionIndices = {nodePosition.x / SPRITE_SCALE, nodePosition.y / SPRITE_SCALE};
		pathQueue.pop();

		std::string test_e = WorldSystem::GameScene->m_Map[(int) nodePositionIndices.y][(int) nodePositionIndices.x];

		if (test_e != EMPTY_CELL) {
			if (isGoalTile(test_e)) {
				m_Path = test_path;
				m_GoalReached = true;
				delete[] visited;
				return;
			}
			// if we run into a ground tile or grass tile do not bother expanding this path further
			if (isTile(test_e)) {
				continue;
			}
		}

		for (vec2 direction : BFS_DIRECTIONS) {
			vec2 new_node = {nodePosition.x + (direction.x * SPRITE_SCALE),
							 nodePosition.y + (direction.y * SPRITE_SCALE)};
			vec2 new_node_indices = nodePositionIndices + direction;

			if (new_node_indices.y >= rows || new_node_indices.x >= cols || new_node_indices.y < 0 ||
				new_node_indices.x < 0) {
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

BehaviorTree::State MoveToGoal::process(ECS_ENTT::Entity e)
{
	auto& motion = e.GetComponent<Motion>();

	m_IterationNum++;
	if (m_IterationNum <= MOVEMENT_DELAY) {
		return BehaviorTree::State::Running;
	}
	m_IterationNum = 0;
	if (m_Path.empty()) {
		return BehaviorTree::State::Successful;
	}

	// TODO: smoother movement along goal path instead of directly updating position
	motion.position = vec3(m_Path.front(), motion.position.z);
	m_Path.pop();
	return BehaviorTree::State::Running;
}

bool MoveToGoal::isGoalTile(std::string s)
{
	return s == T1;
}

bool MoveToGoal::isTile(std::string s)
{
	return s == T2 || s == T3;
}
