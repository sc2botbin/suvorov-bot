// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#include "Dispatcher.h"
#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "core/Map.h"
#include "core/Timer.h"
#include "plugins/ChatterBox.h"
#include "plugins/Diagnosis.h"
#include "plugins/ExtraDrone.h"
#include "plugins/Miner.h"
#include "plugins/RepairMan.h"
#include "plugins/OverlordFarm.h"
#include "plugins/QuarterMaster.h"
#include "plugins/QueenKeeper.h"
#include "plugins/WarpSmith.h"
#include "strategies/protoss/ChargelotPush.h"
#include "strategies/terran/MarinePush.h"
#include "strategies/zerg/HatchFirst.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

// ------------------------------------------------------------------
// ------------------------------------------------------------------
namespace
{
Historican gHistory("dispatcher");
}  // namespace

// ------------------------------------------------------------------
// ------------------------------------------------------------------
Dispatcher::Dispatcher(const std::string& opponent_id_)
: m_builder(new Builder())
{
    gAPI.reset(new API::Interface(Actions(), Control(), Debug(), Observation(), Query()));
    m_plugins.reserve(10);

    if (opponent_id_.empty())
        return;

    gHistory.info() << "Playing against an opponent with id " << opponent_id_ << '\n';
}

// ------------------------------------------------------------------
void Dispatcher::OnGameStart()
{
    m_plugins.clear();
    gHistory.info() << "New game started!\n";

    // NOTE (alkurbatov): Either we are not allowed to retrieve names
    // or this is another bug.
    // gHistory.info() << "Enemy name: " << gAPI->observer().GetEnemyName() << '\n';

    sc2::Race current_race = gAPI->observer().GetCurrentRace();
    Expansions expansions = CalculateExpansionLocations();
    while(expansions.size() <= 1)
    {
        expansions = CalculateExpansionLocations();
    }
    gHub.reset(new Hub(current_race, expansions));

    m_plugins.emplace_back(new Miner());
    m_plugins.emplace_back(new QuarterMaster());
    m_plugins.emplace_back(new RepairMan());
    m_plugins.emplace_back(new ChatterBox());

    // FIXME (alkurbatov): Implement smarter strategy picker.
    if (current_race == sc2::Race::Protoss)
    {
        m_plugins.emplace_back(new WarpSmith());
        m_plugins.emplace_back(new ChargelotPush());
    }
    else if (current_race == sc2::Race::Terran)
    {
        m_plugins.emplace_back(new MarinePush());
    }
    else if (current_race == sc2::Race::Zerg)
    {
        m_plugins.emplace_back(new ExtraDrone());
        m_plugins.emplace_back(new HatchFirst());
        m_plugins.emplace_back(new QueenKeeper());
        m_plugins.emplace_back(new OverlordFarm());
    }

#ifndef BUILD_FOR_LADDER
    m_plugins.emplace_back(new Diagnosis());
#endif

    for (const auto& i : m_plugins)
    {
        i->OnGameStart(m_builder.get());
    }

}

// ------------------------------------------------------------------
void Dispatcher::OnGameEnd()
{
    gHistory.info() << "Game over!\n";

    for (const auto& i : m_plugins)
    {
        i->OnGameEnd();
    }
}

// ------------------------------------------------------------------
void Dispatcher::OnBuildingConstructionComplete(const sc2::Unit* building_)
{
    gHistory.info() << sc2::UnitTypeToName(building_->unit_type) <<
        "(" << building_->tag << ") constructed\n";
}

// ------------------------------------------------------------------
void Dispatcher::OnStep()
{
    Timer clock;

    gHub->OnStep();

    for (const auto& i : m_plugins)
    {
        i->OnStep(m_builder.get());
    }

    m_builder->OnStep();
}

// ------------------------------------------------------------------
void Dispatcher::OnUnitCreated(const sc2::Unit* unit_)
{
    if (unit_->alliance != sc2::Unit::Alliance::Self)
    {
        return;
    }

    if (unit_->display_type == sc2::Unit::DisplayType::Placeholder)
    {
        gHistory.info() <<"Placeholder of " << sc2::UnitTypeToName(unit_->unit_type) << " was detected\n";
        return;
    }

    gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) << "(" << unit_->tag << ") was created\n";

    gHub->OnUnitCreated(*unit_);

    for (const auto& i : m_plugins)
    {
        i->OnUnitCreated(unit_, m_builder.get());
    }

}

// ------------------------------------------------------------------
void Dispatcher::OnUnitIdle(const sc2::Unit* unit_)
{
    gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) << "(" << unit_->tag << ") is idle\n";

    gHub->OnUnitIdle(*unit_);

    for (const auto& i : m_plugins)
    {
        i->OnUnitIdle(unit_, m_builder.get());
    }

}

// ------------------------------------------------------------------
void Dispatcher::OnUnitDamaged(const sc2::Unit* unit_, float health_, float shields_)
{
    for (const auto& i : m_plugins)
    {
        i->OnUnitDamaged(unit_, health_, shields_, m_builder.get());
    }

}

// ------------------------------------------------------------------
void Dispatcher::OnUnitDestroyed(const sc2::Unit* unit_)
{
    if (unit_->alliance != sc2::Unit::Alliance::Self)
    {
        return;
    }

    gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) << "(" << unit_->tag << ") was destroyed\n";

    gHub->OnUnitDestroyed(*unit_);

    for (const auto& i : m_plugins)
    {
        i->OnUnitDestroyed(unit_, m_builder.get());
    }

}

// ------------------------------------------------------------------
void Dispatcher::OnUpgradeCompleted(sc2::UpgradeID id_)
{
    gHistory.info() << sc2::UpgradeIDToName(id_) << " completed\n";

    for (const auto& i : m_plugins)
    {
        i->OnUpgradeCompleted(id_);
    }
}

// ------------------------------------------------------------------
void Dispatcher::OnUnitEnterVision(const sc2::Unit* unit_)
{
    gHub->OnUnitEnterVision(*unit_);

    for (const auto& i : m_plugins)
    {
        i->OnUnitEnterVision(unit_, m_builder.get());
    }
}

// ------------------------------------------------------------------
void Dispatcher::OnError(const std::vector<sc2::ClientError>& client_errors, const std::vector<std::string>& protocol_errors)
{
    for (const auto i : client_errors)
    {
        gHistory.error() << "Encountered client error: " << static_cast<int>(i) << '\n';
    }

    for (const auto& i : protocol_errors)
    {
        gHistory.error() << "Encountered protocol error: " << i << '\n';
    }

}
