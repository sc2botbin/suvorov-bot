// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#pragma once

#include "Builder.h"
#include "Plugin.h"

struct QuarterMaster : Plugin
{
    QuarterMaster();

    void OnStep(Builder* builder_) final;
    void OnUnitCreated(const sc2::Unit* unit_, Builder*) final;

 private:
    size_t m_frames_to_skip = 10 * 22;  // 10 Seconds * 22 Frames Per Second (~22.4)
    size_t m_skip_until_frame;
};
