#include "Decorator.hpp"


// ----------------------------------------------------------------------------
Decorator::~Decorator()
{
    delete m_child;
}

// ----------------------------------------------------------------------------
void Decorator::SetChild(Node* child_)
{
    m_child = child_;
}

// ----------------------------------------------------------------------------
bool Decorator::HasChild() const
{
    return m_child != nullptr;
}
