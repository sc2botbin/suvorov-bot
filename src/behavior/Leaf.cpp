#include "Leaf.hpp"
#include "Blackboard.hpp"

// ----------------------------------------------------------------------------
Leaf::Leaf()
{
}

// ----------------------------------------------------------------------------
Leaf::Leaf(Blackboard* blackboard_)
    : m_blackboard(blackboard_)
{
}

// ----------------------------------------------------------------------------
void Leaf::SetBlackboard(Blackboard* blackboard_)
{
    m_blackboard = blackboard_;
}
