// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#include "Builder.h"
#include "Historican.h"
#include "Hub.h"
#include "core/API.h"

#include "HatchFirst.h"

// ------------------------------------------------------------------
// ------------------------------------------------------------------
namespace
{
Historican gHistory("strategy.zergling_flood");
}  // namespace

// ------------------------------------------------------------------
// ------------------------------------------------------------------
HatchFirst::HatchFirst()
    : Strategy(12.0f)
    , m_openingBuildOrderState(BuildOrderState::WAIT_TILL_FOOD_AMOUNT)
{
}

// ------------------------------------------------------------------
void HatchFirst::OnGameStart(Builder* builder_)
{
    // NOTE (alkurbatov): Here we use 'ScheduleObligatoryOrder' for creatures
    // in order to support proper build order. The training queue is much faster
    // and always has priority over the construction queue.

    // Hatch first.
    builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::ZERG_OVERLORD);
    builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::ZERG_HATCHERY);
    builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::ZERG_EXTRACTOR);
    builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL);
    builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::ZERG_QUEEN);
    builder_->ScheduleObligatoryOrder(sc2::UNIT_TYPEID::ZERG_OVERLORD);
    builder_->ScheduleObligatoryOrder(sc2::UPGRADE_ID::ZERGLINGMOVEMENTSPEED);
}

// ------------------------------------------------------------------
void HatchFirst::OnUnitCreated(const sc2::Unit* unit_, Builder* builder_)
{
    Strategy::OnUnitCreated(unit_, builder_);

    // Getting the first hatchery down.
    if (m_openingBuildOrderState == BuildOrderState::WAIT_TILL_FOOD_AMOUNT && unit_->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_DRONE && gAPI->observer().GetFoodUsed() >= 17)
    {
        gHistory.info() << "HatchFirst BuildOrder - At 17 food, pausing worker production till hatchery is placed.\n";
        builder_->SetWorkerProductionActive(Builder::WorkerProductionState::PAUSED);
        m_openingBuildOrderState = BuildOrderState::WAIT_HATCHERY_CREATION;
    }

    if(m_openingBuildOrderState == BuildOrderState::WAIT_HATCHERY_CREATION && unit_->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_HATCHERY)
    {
        gHistory.info() << "HatchFirst BuildOrder - Hatchery has been placed, unpausing worker production.\n";
        builder_->SetWorkerProductionActive(Builder::WorkerProductionState::ACTIVE);
        m_openingBuildOrderState = BuildOrderState::FINISHED;
    }
}

// ------------------------------------------------------------------
void HatchFirst::OnUnitIdle(const sc2::Unit* unit_, Builder* builder_)
{
    if (unit_->unit_type.ToType() != sc2::UNIT_TYPEID::ZERG_LARVA)
    {
        return;
    }

    uint64_t ordered_lings = static_cast<uint64_t>(builder_->CountScheduledOrders(sc2::UNIT_TYPEID::ZERG_ZERGLING));

    if (gHub->GetLarvas().Count() <= ordered_lings)
    {
        return;
    }

    builder_->ScheduleOptionalOrder(sc2::UNIT_TYPEID::ZERG_ZERGLING);
    gHistory.info() << "Schedule Zerglings training\n";
}
