// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#include "Historican.h"
#include "Strategy.h"
#include "core/API.h"
#include "core/Helpers.h"

#include <sc2api/sc2_map_info.h>

#include <algorithm>

// ------------------------------------------------------------------
// ------------------------------------------------------------------
namespace
{
Historican gHistory("strategy");
}  // namespace

// ------------------------------------------------------------------
// ------------------------------------------------------------------
Strategy::Strategy(float attack_limit_)
    : m_attack_limit(attack_limit_)
{
}

// ------------------------------------------------------------------
void Strategy::OnStep(Builder*)
{
    // Clean up dead bodies.
    auto it = std::remove_if(m_units.begin(), m_units.end(),
        [](const sc2::Unit* unit_) {
            return !unit_->is_alive;
        });

    m_units.erase(it, m_units.end());

    // Waiting until we have enough units based on desired strategy attack limit.
    if (static_cast<float>(m_units.size()) < m_attack_limit)
    {
        return;
    }

    // Find the enemy start location, all attack that.
    auto targets = gAPI->observer().GameInfo().enemy_start_locations;
    gAPI->action().Attack(m_units, targets.front());

    // Remove units once we have sent that group, and increase the strategy attack limit.
    m_units.clear();
    m_attack_limit = std::min<float>(m_attack_limit * 1.5f, 170.0f);
}

// ------------------------------------------------------------------
void Strategy::OnUnitCreated(const sc2::Unit* unit_, Builder*)
{
    // Only adding combat units to our list.

    if (!IsCombatUnit()(*unit_))
    {
        return;
    }

    gHistory.info() << sc2::UnitTypeToName(unit_->unit_type) << " added to attack group\n";
    m_units.push_back(unit_);
}
