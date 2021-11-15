#pragma once

#include "GameObject.h"
#include "behavior/BehaviorTree.hpp"
#include "core/Order.h"

struct QueenObject : GameObject
{
    explicit QueenObject(const sc2::Unit& unit_);

    void SetHomeHatchery(const sc2::Tag tag_);
    void InjectHatchery() const;
    void InjectHatchery(const sc2::Unit& target_) const;

    Node::Status RunBehaviorTree();

 private:
    BehaviorTree* m_behavior_tree = nullptr;
    sc2::Tag m_home_hatchery_tag;
};
