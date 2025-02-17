// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#include "Historican.h"
#include "Hub.h"
#include "core/API.h"
#include "core/Helpers.h"

#include <sc2api/sc2_unit_filters.h>

#include <algorithm>
#include <cmath>

// ------------------------------------------------------------------
// ------------------------------------------------------------------
namespace
{
Historican gHistory("hub");

struct SortByDistance
{
    explicit SortByDistance(const sc2::Point3D& point_);

    bool operator()(const Expansion& lhs_, const Expansion& rhs_) const;

 private:
    sc2::Point3D m_point;
};

// ------------------------------------------------------------------
SortByDistance::SortByDistance(const sc2::Point3D& point_)
    : m_point(point_)
{
}

// ------------------------------------------------------------------
bool SortByDistance::operator()(const Expansion& lhs_, const Expansion& rhs_) const
{
    return sc2::DistanceSquared2D(lhs_.town_hall_location, m_point) < sc2::DistanceSquared2D(rhs_.town_hall_location, m_point);
}

}  // namespace

// ------------------------------------------------------------------
// ------------------------------------------------------------------
Hub::Hub(sc2::Race current_race_, const Expansions& expansions_)
    : m_current_race(current_race_), m_expansions(expansions_)
    , m_current_supply_type(sc2::UNIT_TYPEID::INVALID)
    , m_current_worker_type(sc2::UNIT_TYPEID::INVALID)
 {
    // Closest expansions at the front.
    std::sort(m_expansions.begin(), m_expansions.end(), SortByDistance(gAPI->observer().StartingLocation()));

    // Set worker and supply type.
    switch (m_current_race)
    {
        case sc2::Race::Protoss:
        {
            m_current_supply_type = sc2::UNIT_TYPEID::PROTOSS_PYLON;
            m_current_worker_type = sc2::UNIT_TYPEID::PROTOSS_PROBE;
            return;
        }

        case sc2::Race::Terran:
        {
            m_current_supply_type = sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT;
            m_current_worker_type = sc2::UNIT_TYPEID::TERRAN_SCV;
            return;
        }

        case sc2::Race::Zerg:
        {
            m_current_supply_type = sc2::UNIT_TYPEID::ZERG_OVERLORD;
            m_current_worker_type = sc2::UNIT_TYPEID::ZERG_DRONE;
            return;
        }

        default:
        {
            return;
        }
    }
}

// ------------------------------------------------------------------
void Hub::OnStep()
{
}

// ------------------------------------------------------------------
void Hub::OnUnitCreated(const sc2::Unit& unit_)
{
    switch (unit_.unit_type.ToType())
    {
        // Storing newly created workers.
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE:
        {
            m_free_workers.Add(Worker(unit_));
            return;
        }

        // Storing newly created zerg larva.
        case sc2::UNIT_TYPEID::ZERG_LARVA:
        {
            m_larva.Clear();

            Units larvas = gAPI->observer().GetUnits(sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_LARVA));
            for(const sc2::Unit* larva : larvas())
            {
                m_larva.Add(GameObject((*larva)));
            }

            return;
        }

        // Storing newly created refineries.
        case sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR:
        case sc2::UNIT_TYPEID::PROTOSS_ASSIMILATORRICH:
        case sc2::UNIT_TYPEID::TERRAN_REFINERY:
        case sc2::UNIT_TYPEID::TERRAN_REFINERYRICH:
        case sc2::UNIT_TYPEID::ZERG_EXTRACTOR:
        case sc2::UNIT_TYPEID::ZERG_EXTRACTORRICH:
        {
            Geyser obj(unit_);

            if (m_captured_geysers.Remove(obj))  // might be claimed geyser
            {
                gHistory.info() << "Release claimed geyser\n";
            }

            m_captured_geysers.Add(obj);
            gHistory.info() << "Capture object " << sc2::UnitTypeToName(unit_.unit_type) << '\n';
            return;
        }

        // Storing newly created Townhall.
        case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        case sc2::UNIT_TYPEID::ZERG_HATCHERY:
        {
            CaptureExpansion(unit_);
            gHistory.info() << "Capture region: (" << unit_.pos.x << ", " << unit_.pos.y << ")\n";
            return;
        }

        default:
            return;
    }
}

// ------------------------------------------------------------------
void Hub::OnUnitDestroyed(const sc2::Unit& unit_)
{
    switch (unit_.unit_type.ToType())
    {
        // A worker was killed.
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE:
        {
            if (m_free_workers.Remove(Worker(unit_)))
            {
                return;
            }

            m_busy_workers.Remove(Worker(unit_));
            gHistory.info() << "Our busy worker was destroyed\n";

            // Was this busy worker trying to build an expansion?
            auto it = std::find_if(m_expansions.begin(), m_expansions.end(),
                [unit_](const Expansion& expansion_)
                {
                    return expansion_.worker_tag == unit_.tag;
                });

            // Can't find an expansion.
            if (it == m_expansions.end())
            {
                return;
            }

            // was enroute to build TownHall
            if (it->owner == Owner::CONTESTED)  
            {
                it->RemoveOwner();
            }

            if (it->owner == Owner::SELF)
            {
                // TownHall still under construction
                // NOTE (impulsecloud): decide whether to cancel or send new worker
                // may need military escort to clear region
            }

            return;
        }

        // A Larva was killed.
        case sc2::UNIT_TYPEID::ZERG_LARVA:
        {
            m_larva.Remove(GameObject(unit_));
            return;
        }

        // A Refinery was killed.
        case sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR:
        case sc2::UNIT_TYPEID::TERRAN_REFINERY:
        case sc2::UNIT_TYPEID::ZERG_EXTRACTOR:
        {
            if (m_captured_geysers.Remove(Geyser(unit_)))
            {
                gHistory.info() << "Release object " << sc2::UnitTypeToName(unit_.unit_type) << '\n';
            }

            return;
        }

        // A Townhall was killed.
        case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
        case sc2::UNIT_TYPEID::ZERG_HATCHERY:
        case sc2::UNIT_TYPEID::ZERG_HIVE:
        case sc2::UNIT_TYPEID::ZERG_LAIR:
        {
            RemoveExpansionOwner(unit_);
            gHistory.info() << "Lost region: (" << unit_.pos.x << ", " << unit_.pos.y << ")\n";
            return;
        }

        default:
        {
            return;
        }

    }
}

// ------------------------------------------------------------------
void Hub::OnUnitIdle(const sc2::Unit& unit_)
{
    switch (unit_.unit_type.ToType())
    {
        // An IDLE worker? 
        case sc2::UNIT_TYPEID::PROTOSS_PROBE:
        case sc2::UNIT_TYPEID::TERRAN_SCV:
        case sc2::UNIT_TYPEID::ZERG_DRONE:
        {
            // An IDLE worker that was thought to be busy should be put in free list. Free list workers will be asked to mine minerals by Miner.
            if (m_busy_workers.Swap(Worker(unit_), m_free_workers))
            {
                gHistory.info() << "Our busy worker has finished task\n";
            }

            return;
        }

        // An IDLE larva?
        // case sc2::UNIT_TYPEID::ZERG_LARVA:
        // {
        //     GameObject obj = GameObject(unit_);
        //     if (!m_larva.IsCached(obj))
        //     {
        //         m_larva.Add(obj);
        //         gHistory.info() << "Picked up an idle larva.\n";
        //     }
        // 
        //     return;
        // }

        default:
        {
            break;
        }

    }
}

// ------------------------------------------------------------------
void Hub::OnUnitEnterVision(const sc2::Unit& unit_)
{
    switch (unit_.unit_type.ToType())
    {
        // A Townhall of some kind came into vision.
        case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
        case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
        case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
        case sc2::UNIT_TYPEID::ZERG_HATCHERY:
        case sc2::UNIT_TYPEID::ZERG_HIVE:
        case sc2::UNIT_TYPEID::ZERG_LAIR:
        {
            EnemyOwnsExpansion(unit_);
            gHistory.info() << "Enemy owns region: (" << unit_.pos.x << ", " << unit_.pos.y << ")\n";
            return;
        }

        default:
        {
            return;
        }

    }
}

// ------------------------------------------------------------------
bool Hub::IsOccupied(const sc2::Unit& unit_) const
{
    return m_captured_geysers.IsCached(Geyser(unit_));
}

// ------------------------------------------------------------------
bool Hub::IsTargetOccupied(const sc2::UnitOrder& order_) const
{
    return m_captured_geysers.IsCached(Geyser(order_));
}

// ------------------------------------------------------------------
void Hub::ClaimObject(const sc2::Unit& unit_)
{
    if (sc2::IsVisibleGeyser()(unit_))
    {
        m_captured_geysers.Add(Geyser(unit_));
        gHistory.info() << "Claim object " << sc2::UnitTypeToName(unit_.unit_type) << '\n';
    }
}

// ------------------------------------------------------------------
sc2::Race Hub::GetCurrentRace() const
{
    return m_current_race;
}

// ------------------------------------------------------------------
Worker* Hub::GetClosestFreeWorker(const sc2::Point2D& location_)
{
    Worker* closest_worker = m_free_workers.GetClosestTo(location_);
    if (!closest_worker)
    {
        return nullptr;
    }

    m_free_workers.Swap(*closest_worker, m_busy_workers);
    return &m_busy_workers.Back();
}

// ------------------------------------------------------------------
sc2::UNIT_TYPEID Hub::GetCurrentSupplyType() const
{
    return m_current_supply_type;
}

// ------------------------------------------------------------------
sc2::UNIT_TYPEID Hub::GetCurrentWorkerType() const
{
    return m_current_worker_type;
}

// ------------------------------------------------------------------
bool Hub::AssignRefineryConstruction(Order* order_, const sc2::Unit* geyser_)
{
    Worker* worker = GetClosestFreeWorker(geyser_->pos);
    if (!worker)
    {
        return false;
    }

    worker->BuildRefinery(order_, geyser_);
    ClaimObject(*geyser_);
    return true;
}

// ------------------------------------------------------------------
sc2::Tag Hub::AssignBuildTask(Order* order_, const sc2::Point2D& point_)
{
    Worker* worker = GetClosestFreeWorker(point_);
    if (!worker)
    {
        return sc2::NullTag;
    }

    worker->Build(order_, point_);
    return worker->Tag();
}

// ------------------------------------------------------------------
void Hub::AssignVespeneHarvester(const sc2::Unit& refinery_)
{
    Worker* worker = GetClosestFreeWorker(refinery_.pos);
    if (!worker)
    {
        return;
    }

    worker->GatherVespene(refinery_);
}

// ------------------------------------------------------------------
bool Hub::AssignLarva(Order* order_)
{
    if (m_larva.Empty())
    {
        return false;
    }

    order_->assignee = m_larva.Back().Tag();
    gAPI->action().Build(*order_);

    m_larva.PopBack();
    return true;
}

// ------------------------------------------------------------------
const Cache<GameObject>&  Hub::GetLarvas() const
{
    return m_larva;
}

// ------------------------------------------------------------------
const Expansions& Hub::GetExpansions() const
{
    return m_expansions;
}

// ------------------------------------------------------------------
Expansion* Hub::GetNextExpansion()
{
    auto it = std::find_if(m_expansions.begin(), m_expansions.end(),
        [](const Expansion& expansion_)
        {
            return expansion_.owner == Owner::NEUTRAL;
        });

    if (it == m_expansions.end())
    {
        return nullptr;
    }

    it->owner = Owner::CONTESTED;
    return &(*it);
}

Expansion* Hub::GetClosestExpansion(const sc2::Point2D& location_, Owner expansionOwner_ /*= Owner::NEUTRAL*/)
{
    auto closest_expansion = m_expansions.end();
    float distance = std::numeric_limits<float>::max();
    for(auto it = m_expansions.begin(); it != m_expansions.end(); ++it)
    {
        if(it->owner == expansionOwner_)
        {
            float d = sc2::DistanceSquared2D(it->town_hall_location, location_);
            if (d >= distance)
            {
                continue;
            }

            distance = d;
            closest_expansion = it;
        }
    }

    if (closest_expansion == m_expansions.end())
    {
        return nullptr;
    }

    return &(*closest_expansion);
}

// ------------------------------------------------------------------
Expansion* Hub::GetClosestExpansionExcluding(const sc2::Point2D& location_, std::vector<Expansion*> exclude_, Owner expansionOwner_ /*= Owner::NEUTRAL*/)
{
    auto closest_expansion = m_expansions.end();
    float distance = std::numeric_limits<float>::max();
    for (auto it = m_expansions.begin(); it != m_expansions.end(); ++it)
    {
        if (it->owner == expansionOwner_)
        {
            bool valid = true;

            float d = sc2::DistanceSquared2D(it->town_hall_location, location_);
            if (d >= distance)
            {
                valid = false;
            }

            for (const Expansion* e : exclude_)
            {
                if (e->town_hall_tag == it->town_hall_tag)
                {
                    valid = false;
                    break;
                }
            }

            if(valid)
            {
                distance = d;
                closest_expansion = it;
            }
        }
    }

    if (closest_expansion == m_expansions.end())
    {
        return nullptr;
    }

    return &(*closest_expansion);
}

// ------------------------------------------------------------------
Expansion* Hub::GetClosestExpansionIncluding(const sc2::Point2D& location_, std::vector<Expansion*> include_, Owner expansionOwner_ /*= Owner::NEUTRAL*/)
{
    if(include_.size() == 0)
    {
        return nullptr;
    }

    auto closest_expansion = (*include_.end());
    float distance = std::numeric_limits<float>::max();
    for (auto it = include_.begin(); it != include_.end(); ++it)
    {
        if((*it)->owner == expansionOwner_)
        {
            float d = sc2::DistanceSquared2D((*it)->town_hall_location, location_);
            if (d >= distance)
            {
                continue;
            }

            distance = d;
            closest_expansion = (*it);
        }
    }

    if (closest_expansion == &(*m_expansions.end()))
    {
        return nullptr;
    }

    return &(*closest_expansion);
}

// ------------------------------------------------------------------
Expansion* Hub::GetExpansionOfTownhall(const sc2::Unit& unit_)
{
    auto it = std::find_if(m_expansions.begin(), m_expansions.end(),
        [unit_](const Expansion& e)
        {
            return std::floor(e.town_hall_location.x) == std::floor(unit_.pos.x) && std::floor(e.town_hall_location.y) == std::floor(unit_.pos.y);
        });

    if (it == m_expansions.end())
    {
        return nullptr;
    }

    return &(*it);
}

// ------------------------------------------------------------------
void Hub::CaptureExpansion(const sc2::Unit& unit_)
{
    Expansion* expansion = GetExpansionOfTownhall(unit_);
    if (!expansion)
    {
        return;
    }

    expansion->SetOwner(unit_, Owner::SELF);
}

// ------------------------------------------------------------------
void Hub::EnemyOwnsExpansion(const sc2::Unit& unit_)
{
    Expansion* expansion = GetExpansionOfTownhall(unit_);
    if (!expansion)
    {
        return;
    }

    expansion->SetOwner(unit_, Owner::ENEMY);
}

// ------------------------------------------------------------------
void Hub::RemoveExpansionOwner(const sc2::Unit& unit_)
{
    auto expansion = GetExpansionOfTownhall(unit_);
    if (!expansion)
        return;

    expansion->RemoveOwner();
}

std::unique_ptr<Hub> gHub;
