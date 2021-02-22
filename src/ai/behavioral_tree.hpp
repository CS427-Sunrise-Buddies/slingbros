#pragma once

#include "Entity.h"

namespace BehaviorTree
{
	enum class State
	{
		Running,
		Successful,
		Failure
	};

	class Node
	{
	public:
		virtual ~Node() noexcept = default;

		virtual void init(ECS_ENTT::Entity e)
		{};

		virtual State process(ECS_ENTT::Entity e) = 0;
	};

	// Composite node that succeeds when ALL children succeed
	class Sequence : public Node
	{
	public:
		Sequence(std::vector<Node*> children);

	private:
		void init(ECS_ENTT::Entity e) override;

		State process(ECS_ENTT::Entity e) override;

		size_t m_index;
		std::vector<Node*> m_children;
	};

	// Composite node that succeeds when ANY children succeeds
	class Selector : public Node
	{
	public:
		Selector(std::vector<Node*> children);

	private:
		void init(ECS_ENTT::Entity e) override;

		State process(ECS_ENTT::Entity e) override;

		size_t m_index;
		std::vector<Node*> m_children;
	};
}
