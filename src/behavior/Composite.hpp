#pragma once
#include "Node.hpp"


class Composite : public Node
{

public:

	virtual ~Composite();

	void AddChild(Node* childNode_);
	bool HasChildren() const;
	int GetRunningChildIndex();

protected:

	int m_runningChildIndex = 0;
	std::vector<Node*> m_children;

};
