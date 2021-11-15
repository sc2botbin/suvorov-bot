// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#pragma once

#include "core/Order.h"

#include <list>

#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_unit.h>

struct Builder
{
    Builder();

    void OnStep();

    void OnUnitCreated(const sc2::Unit& unit_);

    // NOTE (alkurbatov): Obligatory orders are primarily executed
    // according to order in the queue.
    void ScheduleObligatoryOrder(sc2::UNIT_TYPEID id_, bool urgent = false);

    // NOTE (alkurbatov): Optional orders are executed if we have
    // free resources available. As many as possible orders will be executed.
    void ScheduleOptionalOrder(sc2::UNIT_TYPEID id_, const sc2::Unit* assignee_ = nullptr);
    void ScheduleObligatoryOrder(sc2::UPGRADE_ID id_);
    void ScheduleOptionalOrder(sc2::UPGRADE_ID id_);
    std::list<Order> GetOrders() const;
    int64_t CountScheduledOrders(sc2::UNIT_TYPEID id_) const;
    void RemoveOrdersOfType(const sc2::UNIT_TYPEID type_);

    // Worker Production.
    enum class WorkerProductionState
    {
        ACTIVE,
        PAUSED,
    };
    void SetWorkerProductionActive(WorkerProductionState workerProductionActive_);
    inline WorkerProductionState GetWorkerProductionActive() { return m_workerProductionActive; }

    // Army Production
    enum class ArmyProductionState
    {
        ACTIVE,
        PAUSED,
    };
    inline void SetArmyProductionActive(ArmyProductionState armyProductionActive_) { m_armyProductionActive = armyProductionActive_; }
    inline ArmyProductionState GetArmyProductionActive() { return m_armyProductionActive; }

 private:
    bool Build(Order* order_);

    uint32_t m_minerals;
    uint32_t m_vespene;

    float m_available_food;

    std::list<Order> m_must_do;
    std::list<Order> m_nice_to_have;

    WorkerProductionState m_workerProductionActive = WorkerProductionState::ACTIVE;
    ArmyProductionState m_armyProductionActive = ArmyProductionState::ACTIVE;
};
