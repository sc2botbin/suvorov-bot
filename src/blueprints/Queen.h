// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#pragma once

#include "Blueprint.h"

struct Queen: Blueprint
{
    bool Build(Order* order_) final;
};
