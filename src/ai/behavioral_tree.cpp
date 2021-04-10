#include "behavioral_tree.hpp"

namespace BehaviorTree
{
	Sequence::Sequence(std::vector<Node*> children)
			: m_index(0), m_children(std::move(children))
	{

	}

	void Sequence::init(ECS_ENTT::Entity e)
	{
		m_index = 0;
		assert(m_index < m_children.size());
		const auto& child = m_children[m_index];
		assert(child);
		child->init(e);
	}

	State Sequence::process(ECS_ENTT::Entity e, float elapsed_ms)
	{
		assert(m_index < m_children.size());
		const auto& child = m_children[m_index];
		assert(child);
		State state = child->process(e, elapsed_ms);

		if (state == State::Successful) {
			++m_index;
			if (m_index >= m_children.size()) {
				// Succeed if all children have succeeded
				return State::Successful;
			} else {
				const auto& nextChild = m_children[m_index];
				assert(nextChild);
				nextChild->init(e);
				return State::Running;
			}
		} else {
			return state;
		}
	}

	Selector::Selector(std::vector<Node*> children)
			: m_index(0), m_children(std::move(children))
	{

	}

	void Selector::init(ECS_ENTT::Entity e)
	{
		m_index = 0;
		assert(m_index < m_children.size());
		const auto& child = m_children[m_index];
		assert(child);
		child->init(e);
	}

	State Selector::process(ECS_ENTT::Entity e, float elapsed_ms)
	{
		assert(m_index < m_children.size());
		const auto& child = m_children[m_index];
		assert(child);
		State state = child->process(e, elapsed_ms);

		if (state == State::Successful) {
			// Succeed if any of the children succeed
			return State::Successful;
		}

		if (state == State::Running) {
			return state;
		}

		// state is State::Failure
		m_index++;
		if (m_index < m_children.size()) {
			const auto& nextChild = m_children[m_index];
			assert(nextChild);
			nextChild->init(e);
			return State::Running;
		} else {
			return State::Failure;
		}
	}
}
