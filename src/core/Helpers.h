// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#pragma once

#include "Order.h"
#include "core/API.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <set>

struct MultiFilter
{
    enum class Selector
    {
        And,
        Or
    };

    MultiFilter(Selector selector_, std::initializer_list<sc2::Filter> filters_);

    bool operator()(const sc2::Unit& unit_) const;

private:
    std::vector<sc2::Filter> m_filters;
    Selector m_selector;
};

struct IsUnit {
    explicit IsUnit(sc2::UNIT_TYPEID type_);

    bool operator()(const sc2::Unit& unit_) const;

 private:
    sc2::UNIT_TYPEID m_type;
};

struct OneOfUnits {
    explicit OneOfUnits(const std::set<sc2::UNIT_TYPEID>& types_);

    bool operator()(const sc2::Unit& unit_) const;

 private:
    const std::set<sc2::UNIT_TYPEID> m_types;
};

struct IsCombatUnit {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsCombatUnitType {
    bool operator()(const sc2::UNIT_TYPEID& id_) const;
};

struct CanAttackAir {
    bool operator()(const sc2::Unit& unit_) const;
};

struct CanAttackAirUnitType {
    bool operator()(const sc2::UNIT_TYPEID& id_) const;
};

struct IsFoggyResource {
    bool operator()(const sc2::Unit& unit_) const;
};

// Check that the provided unit is not occupied and not depleted geyser
struct IsFreeGeyser {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsRefinery {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsIdleUnit {
    explicit IsIdleUnit(sc2::UNIT_TYPEID type_);

    bool operator()(const sc2::Unit& unit_) const;

 private:
    sc2::UNIT_TYPEID m_type;
};

struct IsGasWorker {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsRepairer {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsIdleTownHall {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsCommandCenter {
    bool operator()(const sc2::Unit& unit_) const;
};

struct IsWithinRange {
    explicit IsWithinRange(sc2::Point3D point_, float min_ = std::numeric_limits<float>::max());

    bool operator()(const sc2::Unit& unit_) const;

private:
    sc2::Point3D m_point;
    float m_min;
};

struct IsOrdered {
    explicit IsOrdered(sc2::UNIT_TYPEID type_);

    bool operator()(const Order& order_) const;

 private:
    sc2::UNIT_TYPEID m_type;
};
