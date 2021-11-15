// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#include "Converter.h"
#include "Helpers.h"
#include "Hub.h"

#include <sc2api/sc2_unit_filters.h>

IsUnit::IsUnit(sc2::UNIT_TYPEID type_): m_type(type_) {
}

bool IsUnit::operator()(const sc2::Unit& unit_) const {
    return unit_.unit_type == m_type && unit_.IsBuildFinished();
}

OneOfUnits::OneOfUnits(const std::set<sc2::UNIT_TYPEID>& types_): m_types(types_) {
}

bool OneOfUnits::operator()(const sc2::Unit& unit_) const {
    return unit_.IsBuildFinished() &&
        m_types.find(unit_.unit_type) != m_types.end();
}

bool IsCombatUnit::operator()(const sc2::Unit& unit_) const {

    return IsCombatUnitType()(unit_.unit_type.ToType());
}

bool IsCombatUnitType::operator()(const sc2::UNIT_TYPEID& id_) const
{
    switch (id_) {
    case sc2::UNIT_TYPEID::TERRAN_BANSHEE:
    case sc2::UNIT_TYPEID::TERRAN_CYCLONE:
    case sc2::UNIT_TYPEID::TERRAN_GHOST:
    case sc2::UNIT_TYPEID::TERRAN_HELLION:
    case sc2::UNIT_TYPEID::TERRAN_HELLIONTANK:
    case sc2::UNIT_TYPEID::TERRAN_LIBERATOR:
    case sc2::UNIT_TYPEID::TERRAN_LIBERATORAG:
    case sc2::UNIT_TYPEID::TERRAN_MARAUDER:
    case sc2::UNIT_TYPEID::TERRAN_MARINE:
    case sc2::UNIT_TYPEID::TERRAN_MEDIVAC:
    case sc2::UNIT_TYPEID::TERRAN_RAVEN:
    case sc2::UNIT_TYPEID::TERRAN_REAPER:
    case sc2::UNIT_TYPEID::TERRAN_SIEGETANK:
    case sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
    case sc2::UNIT_TYPEID::TERRAN_THOR:
    case sc2::UNIT_TYPEID::TERRAN_THORAP:
    case sc2::UNIT_TYPEID::TERRAN_VIKINGASSAULT:
    case sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
    case sc2::UNIT_TYPEID::TERRAN_WIDOWMINE:
    case sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED:

    case sc2::UNIT_TYPEID::ZERG_BANELING:
    case sc2::UNIT_TYPEID::ZERG_BANELINGBURROWED:
    case sc2::UNIT_TYPEID::ZERG_BROODLORD:
    case sc2::UNIT_TYPEID::ZERG_CORRUPTOR:
    case sc2::UNIT_TYPEID::ZERG_HYDRALISK:
    case sc2::UNIT_TYPEID::ZERG_HYDRALISKBURROWED:
    case sc2::UNIT_TYPEID::ZERG_INFESTOR:
    case sc2::UNIT_TYPEID::ZERG_INFESTORBURROWED:
    case sc2::UNIT_TYPEID::ZERG_INFESTORTERRAN:
    case sc2::UNIT_TYPEID::ZERG_LURKERMP:
    case sc2::UNIT_TYPEID::ZERG_LURKERMPBURROWED:
    case sc2::UNIT_TYPEID::ZERG_MUTALISK:
    case sc2::UNIT_TYPEID::ZERG_RAVAGER:
    case sc2::UNIT_TYPEID::ZERG_ROACH:
    case sc2::UNIT_TYPEID::ZERG_ROACHBURROWED:
    case sc2::UNIT_TYPEID::ZERG_ULTRALISK:
    case sc2::UNIT_TYPEID::ZERG_VIPER:
    case sc2::UNIT_TYPEID::ZERG_ZERGLING:
    case sc2::UNIT_TYPEID::ZERG_ZERGLINGBURROWED:

    case sc2::UNIT_TYPEID::PROTOSS_ADEPT:
    case sc2::UNIT_TYPEID::PROTOSS_ADEPTPHASESHIFT:
    case sc2::UNIT_TYPEID::PROTOSS_ARCHON:
    case sc2::UNIT_TYPEID::PROTOSS_CARRIER:
    case sc2::UNIT_TYPEID::PROTOSS_COLOSSUS:
    case sc2::UNIT_TYPEID::PROTOSS_DARKTEMPLAR:
    case sc2::UNIT_TYPEID::PROTOSS_DISRUPTOR:
    case sc2::UNIT_TYPEID::PROTOSS_DISRUPTORPHASED:
    case sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR:
    case sc2::UNIT_TYPEID::PROTOSS_IMMORTAL:
    case sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP:
    case sc2::UNIT_TYPEID::PROTOSS_ORACLE:
    case sc2::UNIT_TYPEID::PROTOSS_PHOENIX:
    case sc2::UNIT_TYPEID::PROTOSS_SENTRY:
    case sc2::UNIT_TYPEID::PROTOSS_STALKER:
    case sc2::UNIT_TYPEID::PROTOSS_TEMPEST:
    case sc2::UNIT_TYPEID::PROTOSS_VOIDRAY:
    case sc2::UNIT_TYPEID::PROTOSS_ZEALOT:
        return true;

    default:
        return false;
    }
}

bool CanAttackAir::operator()(const sc2::Unit& unit_) const {

    return CanAttackAirUnitType()(unit_.unit_type.ToType());
}

bool CanAttackAirUnitType::operator()(const sc2::UNIT_TYPEID& id_) const
{
    switch (id_) {
    case sc2::UNIT_TYPEID::TERRAN_CYCLONE:
    case sc2::UNIT_TYPEID::TERRAN_GHOST:
    case sc2::UNIT_TYPEID::TERRAN_LIBERATOR:
    case sc2::UNIT_TYPEID::TERRAN_LIBERATORAG:
    case sc2::UNIT_TYPEID::TERRAN_MARINE:
    case sc2::UNIT_TYPEID::TERRAN_THOR:
    case sc2::UNIT_TYPEID::TERRAN_THORAP:
    case sc2::UNIT_TYPEID::TERRAN_VIKINGASSAULT:
    case sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED:

    case sc2::UNIT_TYPEID::ZERG_CORRUPTOR:
    case sc2::UNIT_TYPEID::ZERG_HYDRALISK:
    case sc2::UNIT_TYPEID::ZERG_HYDRALISKBURROWED:
    case sc2::UNIT_TYPEID::ZERG_INFESTORTERRAN:
    case sc2::UNIT_TYPEID::ZERG_MUTALISK:
    case sc2::UNIT_TYPEID::ZERG_RAVAGER:
    case sc2::UNIT_TYPEID::ZERG_QUEEN:

    
    case sc2::UNIT_TYPEID::PROTOSS_ARCHON:
    case sc2::UNIT_TYPEID::PROTOSS_CARRIER:
    case sc2::UNIT_TYPEID::PROTOSS_COLOSSUS:
    case sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP:
    case sc2::UNIT_TYPEID::PROTOSS_PHOENIX:
    case sc2::UNIT_TYPEID::PROTOSS_SENTRY:
    case sc2::UNIT_TYPEID::PROTOSS_STALKER:
    case sc2::UNIT_TYPEID::PROTOSS_TEMPEST:
    case sc2::UNIT_TYPEID::PROTOSS_VOIDRAY:
        return true;

    default:
        return false;
    }
}

bool IsFoggyResource::operator()(const sc2::Unit& unit_) const {
    if (!sc2::IsGeyser()(unit_) && !sc2::IsMineralPatch()(unit_))
        return false;

    return unit_.display_type != sc2::Unit::DisplayType::Visible;
}

bool IsFreeGeyser::operator()(const sc2::Unit& unit_) const {
    return sc2::IsVisibleGeyser()(unit_) && !gHub->IsOccupied(unit_);
}

bool IsRefinery::operator()(const sc2::Unit& unit_) const {
    if (!unit_.IsBuildFinished())
        return false;

    return unit_.unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
        unit_.unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATORRICH ||
        unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERY ||
        unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERYRICH ||
        unit_.unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTOR ||
        unit_.unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTORRICH;
}

IsIdleUnit::IsIdleUnit(sc2::UNIT_TYPEID type_): m_type(type_) {
}

bool IsIdleUnit::operator()(const sc2::Unit& unit_) const {
    return IsUnit(m_type)(unit_) && unit_.orders.empty();
}

bool IsGasWorker::operator()(const sc2::Unit& unit_) const {
    if (!sc2::IsWorker()(unit_))
        return false;

    if (unit_.orders.empty())
        return false;

    if (unit_.orders.front().ability_id == sc2::ABILITY_ID::HARVEST_RETURN) {
        if (unit_.buffs.empty())
            return false;

        return unit_.buffs.front() == sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGAS ||
            unit_.buffs.front() == sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGASZERG ||
            unit_.buffs.front() == sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGASPROTOSS;
    }

    if (unit_.orders.front().ability_id == sc2::ABILITY_ID::HARVEST_GATHER)
        return gHub->IsTargetOccupied(unit_.orders.front());

    return false;
}

bool IsRepairer::operator()(const sc2::Unit& unit_) const {
    if (!sc2::IsWorker()(unit_))
        return false;

    if (unit_.orders.empty())
        return false;

    return unit_.orders.front().ability_id == sc2::ABILITY_ID::EFFECT_REPAIR ||
        unit_.orders.front().ability_id == sc2::ABILITY_ID::EFFECT_REPAIR_SCV;
}

bool IsIdleTownHall::operator()(const sc2::Unit& unit_) const {
    return sc2::IsTownHall()(unit_) &&
        unit_.orders.empty() && unit_.IsBuildFinished();
}

bool IsCommandCenter::operator()(const sc2::Unit& unit_) const {
    return unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING ||
           unit_.unit_type == sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS;
}


IsWithinRange::IsWithinRange(sc2::Point3D point_, float min_ /*= std::numeric_limits<float>::max()*/)
    : m_point(point_)
    , m_min(min_)
{
}

bool IsWithinRange::operator()(const sc2::Unit& unit_) const
{
    return sc2::DistanceSquared2D(unit_.pos, m_point) < m_min;
}

IsOrdered::IsOrdered(sc2::UNIT_TYPEID type_): m_type(type_) {
}

bool IsOrdered::operator()(const Order& order_) const {
    return order_.unit_type_id == m_type;
}

// ----------------------------------------------------------------------------
MultiFilter::MultiFilter(Selector selector_, std::initializer_list<sc2::Filter> filters_)
    : m_selector(selector_)
    , m_filters(filters_)
{
}

bool MultiFilter::operator()(const sc2::Unit& unit_) const
{
    if (m_selector == Selector::And)
    {
        for (auto& filter : m_filters)
        {
            if (!filter(unit_))
            {
                return false;
            }
        }

        return true;
    }
    else if (m_selector == Selector::Or)
    {
        for (auto& filter : m_filters)
        {
            if (filter(unit_))
            {
                return true;
            }
        }

        return false;
    }

    return false;
}
