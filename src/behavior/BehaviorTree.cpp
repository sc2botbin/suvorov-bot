#include "BehaviorTree.hpp"
#include "Blackboard.hpp"

// ----------------------------------------------------------------------------
BehaviorTree::BehaviorTree()
{
    m_blackboard = new Blackboard();
}

// ----------------------------------------------------------------------------
BehaviorTree::~BehaviorTree()
{
    // We may not have created the root node, but we come to own it, loved it, even thought of it as a child.
    delete m_rootNode;
    delete m_blackboard;
}

// ----------------------------------------------------------------------------
Node::Status BehaviorTree::Run()
{
    return m_rootNode->Update();
}

// ----------------------------------------------------------------------------
void BehaviorTree::SetRootNode(Node* rootNode_)
{
    m_rootNode = rootNode_;
}

// ----------------------------------------------------------------------------
Blackboard* BehaviorTree::GetBlackboard()
{
    return m_blackboard;
}
