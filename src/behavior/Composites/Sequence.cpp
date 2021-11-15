#include "Sequence.hpp"


// ----------------------------------------------------------------------------
void Sequence::Start()
{
    // When we start running a Sequence, we start from the left-most side;
    m_runningChildIndex = 0;
}

// ----------------------------------------------------------------------------
Node::Status Sequence::Run()
{
    // Early out, if we have no children then we have nothing to run, return SUCCESS;
    if (!HasChildren())
    {
        return Status::SUCCESS;
    }

    while (true)
    {
        // Get the currently running child;
        Node* child = m_children[m_runningChildIndex];
        Status status = child->Update();

        // Sequence will return when we find our first FAILURE (or RUNNING);
        if (status == Status::FAILURE)
        {
            return Status::FAILURE;
        }
        else if (status == Status::RUNNING)
        {
            return Status::RUNNING;
        }

        // If we did not return, then it means that the child failed and we should move on;
        m_runningChildIndex++;

        // If we make it to the end of our children, and none had FAILURE (or RUNNING) then we as a whole SUCCESS;
        if (m_runningChildIndex == m_children.size())
        {
            return Status::SUCCESS;
        }
    }
}
