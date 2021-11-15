#pragma once
#include "Node.hpp"

class Blackboard;

class BehaviorTree : public Node
{

public:
    BehaviorTree();
    virtual ~BehaviorTree();

    Status Run() override;

    void SetRootNode(Node* rootNode_);
    Blackboard* GetBlackboard();

private:
    Node* m_rootNode = nullptr;
    Blackboard* m_blackboard = nullptr;

};