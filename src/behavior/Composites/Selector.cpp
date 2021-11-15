#include "Selector.hpp"

// ----------------------------------------------------------------------------
void Selector::Start()
{
    // When we start running a Selector, we start from the left-most side;
    m_runningChildIndex = 0;
}

// ----------------------------------------------------------------------------
Node::Status Selector::Run()
{
    // Early out, if we have no children then we have nothing to run, return SUCCESS;
    if(!HasChildren())
    {
        return Status::SUCCESS;
    }

    while(true)
    {
        // Get the currently running child;
        Node* child = m_children[m_runningChildIndex];
        Status status = child->Update();

        // Selector will return when we find our first SUCCESS (or RUNNING);
        if(status == Status::SUCCESS)
        {
            return Status::SUCCESS;
        }
        else if(status == Status::RUNNING)
        {
            return Status::RUNNING;
        }

        // If we did not return, then it means that the child failed and we should move on;
        m_runningChildIndex++;

        // If we make it to the end of our children, and none had SUCCESS (or RUNNING) then we as a whole FAILED;
        if(m_runningChildIndex == m_children.size())
        {
            return Status::FAILURE;
        }
    }
}
