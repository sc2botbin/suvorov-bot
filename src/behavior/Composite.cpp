#include "Composite.hpp"


// ----------------------------------------------------------------------------
Composite::~Composite()
{
    // Even though we do not new off children, we become responsible for them;
    for(Node* child : m_children)
    {
        delete child;
    }
    m_children.clear();
}

// ----------------------------------------------------------------------------
void Composite::AddChild(Node* childNode_)
{
    m_children.push_back(childNode_);
}

// ----------------------------------------------------------------------------
bool Composite::HasChildren() const
{
    return !m_children.empty();
}

// ----------------------------------------------------------------------------
int Composite::GetRunningChildIndex()
{
    return m_runningChildIndex;
}
