// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#include "Historican.h"
#include "Hub.h"
#include "RepairMan.h"
#include "core/API.h"
#include "core/Helpers.h"

#include <sc2api/sc2_unit_filters.h>

// ------------------------------------------------------------------
// ------------------------------------------------------------------
namespace
{
Historican gHistory("repairman");

}  // namespace

// ------------------------------------------------------------------
// ------------------------------------------------------------------
void RepairMan::OnStep(Builder*)
{
    // Terran only!
    if (gHub->GetCurrentRace() != sc2::Race::Terran)
    {
        return;
    }

    if (m_damaged_buildings.empty() || gAPI->observer().GetMinerals() == 0)
    {
        return;
    }

    m_damaged_buildings.remove_if([](const sc2::Unit* unit_)
    {
        return unit_->health == unit_->health_max || !unit_->is_alive;
    });

    if (m_damaged_buildings.empty())
        return;

    auto repairers = gAPI->observer().GetUnits(IsRepairer());

    if (repairers().size() >= 3 )
        return;

    AssignRepairTask(*m_damaged_buildings.front());
}

// ------------------------------------------------------------------
void RepairMan::OnUnitDamaged(const sc2::Unit* unit_, float health_, float, Builder*)
{
    // Terran only!
    if (gHub->GetCurrentRace() != sc2::Race::Terran)
    {
        return;
    }

    // Only check our buildings, we don't care about their buildings being damaged.
    if (!sc2::IsBuilding()(*unit_) || unit_->alliance != sc2::Unit::Alliance::Self)
    {
        return;
    }

    if (unit_->health + health_ < unit_->health_max)
    {
        return;
        // only send repairer the first time it's damaged
    }

    m_damaged_buildings.push_back(unit_);
    AssignRepairTask(*unit_);
}

// ------------------------------------------------------------------
void RepairMan::OnUnitDestroyed(const sc2::Unit* unit_, Builder* builder_)
{
    // Will rebuild destroyed buildings.

    if (IsCombatUnit()(*unit_))
    {
        return;
    }

    // NOTE (alkurbatov): If build_progress < 1.0f a unit might be
    // destroyed by the CancelConstruction command.
    if (!unit_->IsBuildFinished())
    {
        return;
    }

    switch (unit_->unit_type.ToType())
    {
        case sc2::UNIT_TYPEID::PROTOSS_PYLON:
        case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT:
        case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED:
        case sc2::UNIT_TYPEID::ZERG_OVERLORD:
        case sc2::UNIT_TYPEID::ZERG_OVERLORDCOCOON:
        case sc2::UNIT_TYPEID::ZERG_OVERLORDTRANSPORT:
        case sc2::UNIT_TYPEID::ZERG_OVERSEER:
        {
            // NOTE (alkurbatov): QuarterMaster is responsible for supplies rebuild.
            return;
        }

        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE:
        case sc2::UNIT_TYPEID::ZERG_DRONEBURROWED:
        {
            // NOTE (alkurbatov): Miner is responsible for workers rebuild.
            return;
        }

        case sc2::UNIT_TYPEID::TERRAN_MULE:
        case sc2::UNIT_TYPEID::TERRAN_POINTDEFENSEDRONE:
        case sc2::UNIT_TYPEID::TERRAN_KD8CHARGE:
        case sc2::UNIT_TYPEID::ZERG_BROODLING:
        case sc2::UNIT_TYPEID::ZERG_EGG:
        case sc2::UNIT_TYPEID::ZERG_LARVA:
        {
            // Just no.
            return;
        }

        default:
        {
            builder_->ScheduleObligatoryOrder(unit_->unit_type.ToType(), true);
            return;
        }
    }
}

// ------------------------------------------------------------------
void RepairMan::AssignRepairTask(const sc2::Unit& damaged_)
{
    const sc2::Point2D repair_location(damaged_.pos);
    Worker* worker = gHub->GetClosestFreeWorker(repair_location);
    if (!worker)
    {
        return;
    }

    worker->Repair(damaged_);
}
