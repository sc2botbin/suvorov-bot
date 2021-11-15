#include "QueenObject.h"

#include "core/API.h"

// ------------------------------------------------------------------
// ------------------------------------------------------------------
QueenObject::QueenObject(const sc2::Unit& unit_)
    : GameObject(unit_)
    , m_home_hatchery_tag(sc2::NullTag)
{
}

// ------------------------------------------------------------------
void QueenObject::SetHomeHatchery(const sc2::Tag tag_)
{
    m_home_hatchery_tag = tag_;
}

// ------------------------------------------------------------------
void QueenObject::InjectHatchery() const
{
    gAPI->action().Cast(ToUnit(), sc2::ABILITY_ID::EFFECT_INJECTLARVA, GameObject::ToUnit(m_home_hatchery_tag), true);
}

// ------------------------------------------------------------------
void QueenObject::InjectHatchery(const sc2::Unit& target_) const
{
    gAPI->action().Cast(ToUnit(), sc2::ABILITY_ID::EFFECT_INJECTLARVA, target_, true);
}

// ------------------------------------------------------------------
Node::Status QueenObject::RunBehaviorTree()
{
    return m_behavior_tree->Run();
}
