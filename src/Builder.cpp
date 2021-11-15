// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#include "blueprints/Blueprint.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "objects/Worker.h"
#include "Builder.h"
#include "Historican.h"
#include "Hub.h"

#include <algorithm>
#include <memory>

// ------------------------------------------------------------------
// ------------------------------------------------------------------
Builder::Builder()
    : m_minerals(0)
    , m_vespene(0)
    , m_available_food(0.0f)
{
}

// ------------------------------------------------------------------
void Builder::OnStep()
{
    m_minerals = gAPI->observer().GetMinerals();
    m_vespene = gAPI->observer().GetVespene();

    m_available_food = gAPI->observer().GetAvailableFood();

    auto it = m_must_do.begin();
    while (it != m_must_do.end())
    {
        if (!Build(&(*it)))
        {
            break;
        }

        it = m_must_do.erase(it);
    }

    it = m_nice_to_have.begin();
    while (it != m_nice_to_have.end())
    {
        if (!Build(&(*it)))
        {
            ++it;
            continue;
        }

        it = m_nice_to_have.erase(it);
    }
}

// ------------------------------------------------------------------
void Builder::ScheduleObligatoryOrder(sc2::UNIT_TYPEID id_, bool urgent)
{
    // If we are asked to make a worker when something has asked this to be paused, return.
    if (gHub->GetCurrentWorkerType() == id_ && m_workerProductionActive == WorkerProductionState::PAUSED)
    {
        return;
    }

    // If we are asked to make army when something has asked this to be paused, return.
    if (IsCombatUnitType()(id_) && m_armyProductionActive == ArmyProductionState::PAUSED)
    {
        return;
    }

    sc2::UnitTypeData structure = gAPI->observer().GetUnitTypeData(id_);

    // Prevent deadlock.
    if (structure.tech_requirement != sc2::UNIT_TYPEID::INVALID
    && gAPI->observer().CountUnitType(structure.tech_requirement) == 0
    && CountScheduledOrders(structure.tech_requirement) == 0)
    {
        ScheduleObligatoryOrder(structure.tech_requirement);
    }

    if (urgent)
    {
        m_must_do.emplace_front(structure);
        return;
    }

    m_must_do.emplace_back(structure);
}

// ------------------------------------------------------------------
void Builder::ScheduleObligatoryOrder(sc2::UPGRADE_ID id_)
{
    m_must_do.emplace_back(gAPI->observer().GetUpgradeData(id_));
}

// ------------------------------------------------------------------
void Builder::ScheduleOptionalOrder(sc2::UPGRADE_ID id_)
{
    m_nice_to_have.emplace_back(gAPI->observer().GetUpgradeData(id_));
}

// ------------------------------------------------------------------
void Builder::ScheduleOptionalOrder(sc2::UNIT_TYPEID id_, const sc2::Unit* assignee_)
{
    // If we are asked to make a worker when something has asked this to be paused, return.
    if (gHub->GetCurrentWorkerType() == id_ && m_workerProductionActive == WorkerProductionState::PAUSED)
    {
        return;
    }

    m_nice_to_have.emplace_back(gAPI->observer().GetUnitTypeData(id_), assignee_);
}

// ------------------------------------------------------------------
std::list<Order> Builder::GetOrders() const
{
    std::list<Order> all_orders(m_must_do);

    std::copy(
        m_nice_to_have.begin(),
        m_nice_to_have.end(),
        std::back_inserter(all_orders));

    return all_orders;
}

// ------------------------------------------------------------------
int64_t Builder::CountScheduledOrders(sc2::UNIT_TYPEID id_) const
{
    return std::count_if(m_must_do.begin(), m_must_do.end(), IsOrdered(id_))
         + std::count_if(m_nice_to_have.begin(), m_nice_to_have.end(), IsOrdered(id_));
}

void Builder::RemoveOrdersOfType(const sc2::UNIT_TYPEID type_)
{
    m_must_do.remove_if([&type_](const Order order)
    {
        return order.unit_type_id == type_;
    });

    m_nice_to_have.remove_if([&type_](const Order order)
    {
        return order.unit_type_id == type_;
    });
}

void Builder::SetWorkerProductionActive(WorkerProductionState workerProductionActive_)
{
    m_workerProductionActive = workerProductionActive_;
    if (m_workerProductionActive == WorkerProductionState::PAUSED)
    {
        RemoveOrdersOfType(gHub->GetCurrentWorkerType());
    }
}

// ------------------------------------------------------------------
bool Builder::Build(Order* order_)
{
    // TODO Fix the fact that it adds the drone cost to the hatchery cost. It is 350 minerals by default? Should be 300.
    if (order_->tech_requirement == sc2::UNIT_TYPEID::ZERG_HATCHERY)
    {
        order_->mineral_cost = 300;
        order_->vespene_cost = 0;
    }

    // TODO Spawning pool is 300? Should be 200.
    if (order_->tech_requirement == sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL)
    {
        order_->mineral_cost = 200;
        order_->vespene_cost = 0;
    }

    if (m_minerals < order_->mineral_cost || m_vespene < order_->vespene_cost)
    {
        return false;
    }

    std::shared_ptr<Blueprint> blueprint = Blueprint::Plot(order_->ability_id);

    // NOTE (alkurbatov): sc2::UNIT_TYPEID::INVALID means that no tech required.
    if (order_->tech_requirement != sc2::UNIT_TYPEID::INVALID)
    {
        std::set<sc2::UNIT_TYPEID> requirements;
        requirements.insert(order_->tech_requirement);

        if (order_->tech_requirement == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER)
        {
            requirements.insert(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING);
            requirements.insert(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND);
            requirements.insert(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING);
            requirements.insert(sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS);
        }

        if (order_->tech_requirement == sc2::UNIT_TYPEID::ZERG_HATCHERY)
        {
            requirements.insert(sc2::UNIT_TYPEID::ZERG_HIVE);
            requirements.insert(sc2::UNIT_TYPEID::ZERG_LAIR);
        }

        if (gAPI->observer().CountUnitsTypes(requirements) == 0)
        {
            return false;
        }

    }

    if (order_->food_required > 0 && m_available_food < order_->food_required)
    {
        return false;
    }

    if (!blueprint->Build(order_))
    {
        return false;
    }

    m_minerals -= order_->mineral_cost;
    m_vespene -= order_->vespene_cost;
    m_available_food -= order_->food_required;

    Historican("builder").info() << "Started building a " << order_->name << '\n';
    return true;
}
