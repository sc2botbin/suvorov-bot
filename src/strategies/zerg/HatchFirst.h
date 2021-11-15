#pragma once

#include "Builder.h"
#include "strategies/Strategy.h"

struct HatchFirst : Strategy
{
    HatchFirst();

    void OnGameStart(Builder* builder_) final;
    void OnUnitCreated(const sc2::Unit* unit_, Builder* builder_) override;
    void OnUnitIdle(const sc2::Unit* unit_, Builder* builder_) final;

private:
    enum BuildOrderState
    {
        WAIT_TILL_FOOD_AMOUNT,
        WAIT_HATCHERY_CREATION,
        FINISHED,
    };

    BuildOrderState m_openingBuildOrderState;
};
