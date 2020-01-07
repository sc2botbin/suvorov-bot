// The MIT License (MIT)
//
// Copyright (c) 2017-2020 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"

struct QuarterMaster : Plugin {
    QuarterMaster();

    void OnStep(Builder* builder_) final;

    void OnUnitCreated(const sc2::Unit* unit_, Builder*) final;

 private:
    bool m_skip_turn;
};
