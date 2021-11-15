#pragma once
#include "behavior/Composite.hpp"

class Sequence : public Composite
{
    void Start() override;
    Status Run() override;

};