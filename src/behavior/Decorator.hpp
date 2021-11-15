#pragma once
#include "Node.hpp"


class Decorator : public Node
{

public:

	virtual ~Decorator();

	void SetChild(Node* child_);
	bool HasChild() const;

protected:

	Node* m_child = nullptr;

};
