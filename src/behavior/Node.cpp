#include "Node.hpp"

// ------------------------------------------------------------------
// ------------------------------------------------------------------
Node::~Node()
{
}

// ------------------------------------------------------------------
Node::Status Node::Update()
{
    if(m_status != Status::RUNNING)
    {
        Start();
    }

    m_status = Run();

    if(m_status != Status::RUNNING)
    {
        Finish(m_status);
    }

    return m_status;
}

// ------------------------------------------------------------------
bool Node::IsSuccess() const
{
    return m_status == Status::SUCCESS;
}

// ------------------------------------------------------------------
bool Node::IsFailure() const
{
    return m_status == Status::FAILURE;
}

// ------------------------------------------------------------------
bool Node::IsRunning() const
{
    return m_status == Status::RUNNING;
}

// ------------------------------------------------------------------
bool Node::IsFinished() const
{
    return IsSuccess() || IsFailure();
}

// ------------------------------------------------------------------
void Node::Reset()
{
    m_status = Status::INVALID;
}
