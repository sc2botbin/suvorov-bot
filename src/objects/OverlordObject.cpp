#include "OverlordObject.h"

#include "core/API.h"

// ------------------------------------------------------------------
// ------------------------------------------------------------------
OverlordObject::OverlordObject(const sc2::Unit& unit_)
    : GameObject(unit_)
{
}

// ------------------------------------------------------------------
Node::Status OverlordObject::RunBehaviorTree()
{
    return m_behavior_tree->Run();
}
