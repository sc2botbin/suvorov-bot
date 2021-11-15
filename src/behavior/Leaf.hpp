#pragma once
#include "Node.hpp"


class Blackboard;

class Leaf : public Node
{

public:

	Leaf();
	Leaf(Blackboard* blackboard_);

	void SetBlackboard(Blackboard* blackboard_);

	virtual Status Run() = 0;

protected:

	Blackboard* m_blackboard = nullptr;

};
