#pragma once
#include "behavior/Composite.hpp"

class Selector : public Composite
{
    void Start() override;
    Status Run() override;

};
