// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#pragma once

#include "Plugin.h"

struct ChatterBox: Plugin
{
    void OnGameStart(Builder*) final;
    void OnStep(Builder*) final;

public:
    static void SendMessage(const std::string& msg_);
};

