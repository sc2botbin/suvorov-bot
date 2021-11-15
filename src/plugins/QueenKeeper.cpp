#include "Hub.h"
#include "QueenKeeper.h"
#include "plugins/ChatterBox.h"
#include "core/API.h"
#include "core/Helpers.h"

#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_unit_filters.h>

#include <queue>

// ------------------------------------------------------------------
// ------------------------------------------------------------------
namespace
{

}  // namespace

// ------------------------------------------------------------------
// ------------------------------------------------------------------
QueenKeeper::QueenKeeper()
{
}

// ------------------------------------------------------------------
void QueenKeeper::OnStep(Builder* builder_)
{
    // Get count of my bases
    // Get count of queens in production
    // if expansions > queens then build another
    Units town_halls = gAPI->observer().GetUnits(sc2::IsTownHall());

    int64_t queens_in_production = builder_->CountScheduledOrders(sc2::UNIT_TYPEID::ZERG_QUEEN);
    Units queens = gAPI->observer().GetUnits(sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_QUEEN));
    for (const sc2::Unit* queen : queens())
    {
        if (!queen->IsBuildFinished())
        {
            ++queens_in_production;
        }
    }

    // Make queens if not enough are in production.
    sc2::Units completed_town_halls;
    for(const sc2::Unit* town_hall : town_halls())
    {
        if(town_hall->IsBuildFinished())
        {
            completed_town_halls.emplace_back(town_hall);
        }
    }

    if((size_t)queens_in_production < completed_town_halls.size())
    {
        ChatterBox::SendMessage("Building another Queen.");
        builder_->ScheduleOptionalOrder(sc2::UNIT_TYPEID::ZERG_QUEEN);
    }
    //

    // hatchery queens will inject.
    for(const QueenObject& queen : m_hatchery_queens())
    {
        queen.InjectHatchery();
    }

    // We also will run the queens behavior tree from here
//     for(const sc2::Unit* queen : m_queens)
//     {
//         Queen* q = (Queen*)queen;
//         switch(q->RunBehaviorTree())
//         {

//         case Node::Status::RUNNING:
//         {
//             break;
//         }
// 
//         case Node::Status::SUCCESS:
//         {
//             break;
//         }
// 
//         case Node::Status::FAILURE:
//         {
//             break;
//         }
// 
//         default:
//             break;

//         }
//     }

}

// ------------------------------------------------------------------
void QueenKeeper::OnUnitCreated(const sc2::Unit* unit_, Builder*)
{
    if(unit_->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_QUEEN)
    {
        m_free_queens.Add(QueenObject(*unit_));
        RebalanceQueens();
    }
}

// ------------------------------------------------------------------
void QueenKeeper::OnUnitDestroyed(const sc2::Unit* unit_, Builder*)
{
    // Will remove queens from being tracked.

    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_QUEEN)
    {
        m_free_queens.Remove(QueenObject(*unit_));
        m_hatchery_queens.Remove(QueenObject(*unit_));
        RebalanceQueens();
    }
}

// ------------------------------------------------------------------
void QueenKeeper::RebalanceQueens()
{
    bool didSwap = false;

    // TODO The transfer of a queen from one Cache to another will not carry over the HomeHatchery of the Queen. And all other personal information to the object.

    // If the count of Hatchery Queens is less than bases, then pull some out of the free queens.
    std::set<sc2::UNIT_TYPEID> types { sc2::UNIT_TYPEID::ZERG_HATCHERY, sc2::UNIT_TYPEID::ZERG_LAIR, sc2::UNIT_TYPEID::ZERG_HIVE };
    size_t townhall_count = gAPI->observer().CountUnitsTypes(types);
    size_t free_queen_count = (size_t)m_free_queens.Count();
    size_t hatchery_queen_count = (size_t)m_hatchery_queens.Count();
    while(free_queen_count > 0 && hatchery_queen_count < townhall_count)
    {
        m_free_queens.Swap(QueenObject(m_free_queens.Back().ToUnit()), m_hatchery_queens);
        --free_queen_count;
        didSwap = true;
    }

    if(didSwap)
    {
        // Attempt to go through the hatchery_queens and assign free expansions as their home.
        std::vector<Expansion*> expansions;
        for(QueenObject& queen : m_hatchery_queens())
        {
            Expansion* closest_expansion = gHub->GetClosestExpansionExcluding(queen.GetPos(), expansions, Owner::SELF);
            if (closest_expansion)
            {
                expansions.emplace_back(closest_expansion);
                queen.SetHomeHatchery(closest_expansion->town_hall_tag);
            }
        }
    }
}
