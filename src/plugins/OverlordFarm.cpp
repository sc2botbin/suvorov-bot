#include "Hub.h"
#include "OverlordFarm.h"
#include "plugins/ChatterBox.h"
#include "core/API.h"
#include "core/Helpers.h"
#include "core/Map.h"

#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_unit_filters.h>

#include <queue>
#include <math.h>

// ------------------------------------------------------------------
// ------------------------------------------------------------------
namespace
{
    const float PI = 3.1415927f;

    // ------------------------------------------------------------------
    float Length(sc2::Point3D vector_)
    {
        float a2 = vector_.x * vector_.x;
        float b2 = vector_.y * vector_.y;
        float c2 = vector_.z * vector_.z;
        float sum = a2 + b2 + c2;
        return std::sqrt(sum);
    }

    // ------------------------------------------------------------------
    sc2::Point3D Normalize(sc2::Point3D vector_)
    {
        float length = Length(vector_);

        return { (vector_.x / length), (vector_.y / length), (vector_.z / length) };
    }

    void SetAngle(sc2::Point3D& vector, float degree)
    {
        float length = Length(vector);
        float angle = (degree * PI) / 180.0f;
        vector.x = cos(angle);
        vector.y = sin(angle);
        vector *= length;
    }

    // ------------------------------------------------------------------
    sc2::Point3D Truncate(sc2::Point3D vector_, sc2::Point3D max_)
    {
        sc2::Point3D out;
        out.x = vector_.x  > max_.x ? max_.x : vector_.x;
        out.y = vector_.y  > max_.y ? max_.y : vector_.y;
        out.z = vector_.z  > max_.z ? max_.z : vector_.z;
        return out;
    }

    // ------------------------------------------------------------------
    sc2::Point3D Truncate(sc2::Point3D vector_, float max_)
    {
        return Length(vector_) > max_ ? Normalize(vector_) * max_ : vector_;
    }

    // ------------------------------------------------------------------
//     struct SortByDistance
//     {
//         explicit SortByDistance(const sc2::Point3D& point_);
// 
//         bool operator()(const Expansion& lhs_, const Expansion& rhs_) const;
// 
//     private:
//         sc2::Point3D m_point;
//     };

    // ------------------------------------------------------------------
//     SortByDistance::SortByDistance(const sc2::Point3D& point_)
//         : m_point(point_)
//     {
//     }

    // ------------------------------------------------------------------
//     bool SortByDistance::operator()(const Expansion& lhs_, const Expansion& rhs_) const
//     {
//         return sc2::DistanceSquared2D(lhs_.town_hall_location, m_point) < sc2::DistanceSquared2D(rhs_.town_hall_location, m_point);
//     }

    // ------------------------------------------------------------------
    int random_int_in_range(int min, int max) //range : [min, max]
    {
        static bool first = true;
        if (first)
        {
            srand((unsigned int)time(NULL)); //seeding for the first time only!
            first = false;
        }
        return min + rand() % ((max + 1) - min);
    }

}  // namespace

// ------------------------------------------------------------------
// ------------------------------------------------------------------
OverlordFarm::OverlordFarm()
{
    // Assumes two player map to find and sort enemy bases.
    auto targets = gAPI->observer().GameInfo().enemy_start_locations;
    sc2::Point2D enemy_main = targets.front();
    m_enemy_bases = gHub->GetExpansions();
    //std::sort(targets.begin(), targets.end(), SortByDistance({ enemy_main.x, enemy_main.y, 0.0f }));
}

// ------------------------------------------------------------------
void OverlordFarm::OnGameStart(Builder*)
{

}

// ------------------------------------------------------------------
void OverlordFarm::OnStep(Builder*)
{
    for(OverlordObject& overlord : m_overlords())
    {
        sc2::Point3D velocity = overlord.GetVelocity();
        if(overlord.GetJob() == OverlordJob::SCOUTMAP)
        {
            Wander(overlord, velocity);
        }
        else
        {
            Seek(overlord, velocity);
        }
        Avoid(overlord, velocity);
        overlord.SetVelocity(velocity);

        gAPI->action().Move(gAPI->observer().GetUnit(overlord.Tag()), overlord.GetPos() + velocity);
    }
}

// ------------------------------------------------------------------
void OverlordFarm::OnUnitCreated(const sc2::Unit* unit_, Builder*)
{
    if(unit_->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_OVERLORD)
    {
        m_overlords.Add(OverlordObject(*unit_));

        OverlordObject& o = m_overlords.Back();
        auto targets = gAPI->observer().GameInfo().enemy_start_locations;
        sc2::Point2D p = targets.front();
        o.SetDestination(sc2::Point3D(p.x, p.y, 0.0f));
        //UpdateOverlordJobs();
    }
}

// ------------------------------------------------------------------
void OverlordFarm::OnUnitDestroyed(const sc2::Unit* unit_, Builder*)
{
    if (unit_->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_OVERLORD)
    {
        m_overlords.Remove(OverlordObject(*unit_));
        //UpdateOverlordJobs();
    }
}

// ------------------------------------------------------------------
void OverlordFarm::Seek(OverlordObject& overlord_, sc2::Point3D& outVelocity_)
{
    sc2::Point3D desired_velocity = Normalize(overlord_.GetDestination() - overlord_.GetPos()) /* * max_speed*/;
    sc2::Point3D steering = desired_velocity - outVelocity_;
    steering = Truncate(steering, {0.1f, 0.1f, 0.1f}); // max_force
    outVelocity_ = Truncate((outVelocity_ + steering), overlord_.GetMaxSpeed());
}

// ------------------------------------------------------------------
void OverlordFarm::Avoid(OverlordObject& overlord_, sc2::Point3D& outVelocity_)
{
    Units combat_units = gAPI->observer().GetUnits(
        MultiFilter(MultiFilter::Selector::And,
        {
            IsCombatUnit(),
            IsWithinRange(overlord_.GetPos(), 220.0f)
            
        }), sc2::Unit::Enemy);

    if(combat_units.Size() > 0)
    {
        sc2::Point3D center = combat_units.GetClosestUnit(overlord_.GetPos())->pos;
        sc2::Point3D desired_velocity = Normalize(overlord_.GetPos() - center) /* * max_speed*/;
        sc2::Point3D steering = desired_velocity - outVelocity_;
        steering = Truncate(steering, { 1.0f, 1.0f, 1.0f }); // max_force
        outVelocity_ = Truncate((outVelocity_ + steering), overlord_.GetMaxSpeed());
    }
}

// ------------------------------------------------------------------
void OverlordFarm::Wander(OverlordObject& overlord, sc2::Point3D& outVelocity)
{
    sc2::Point3D circle_center = Normalize(overlord.GetVelocity()) * 10.0f;
    sc2::Point3D displacement = sc2::Point3D(0.0f, -1.0f, 0.0f) * 5.0f;
    SetAngle(displacement, overlord.GetWanderAngle());

    int r = random_int_in_range(-1, 1);
    overlord.SetWanderAngle(overlord.GetWanderAngle() + (overlord.GetWanderAngle() * r));

    outVelocity = circle_center + displacement;
}

// ------------------------------------------------------------------
// void OverlordFarm::UpdateOverlordJobs()
// {
    // SCOUTMAIN        1
    // SCOUTNATURAL     1
    // SCOUTTHIRD       1
    // SCOUTMAP         10
    // STANDBY          rest

    // Get a working list of references. We will take and remove overlords for jobs.
//     Cache<OverlordObject> working_list;
//     for(OverlordObject& overlord : m_overlords())
//     {
//         working_list.Add(overlord);
//     }

    // SCOUTMAIN
//     Cache<OverlordObject&> scoutmain = GetOverlordsWithJob(working_list, OverlordJob::SCOUTMAIN);
//     if(scoutmain.Count() == 0)
//     {
//         sc2::Point3D p = m_enemy_bases[0].town_hall_location;
//         OverlordObject* o = m_overlords.GetClosestTo(p);
//         o->SetDestination(sc2::Point3D(p.x, p.y, 0.0f));
//         o->SetJob(OverlordJob::SCOUTMAIN);
//         working_list.Remove(*o);
//     }

    // SCOUTNATURAL
//     Cache<OverlordObject&> scoutnatural = GetOverlordsWithJob(working_list, OverlordJob::SCOUTNATURAL);
//     if (scoutmain.Count() == 0)
//     {
//         sc2::Point3D p = m_enemy_bases[1].town_hall_location;
//         OverlordObject* o = m_overlords.GetClosestTo(p);
//         o->SetDestination(sc2::Point3D(p.x, p.y, 0.0f));
//         o->SetJob(OverlordJob::SCOUTNATURAL);
//         working_list.Remove(*o);
//     }

    // SCOUTTHIRD
//     Cache<OverlordObject&> scoutnatural = GetOverlordsWithJob(working_list, OverlordJob::SCOUTTHIRD);
//     if (scoutmain.Count() == 0)
//     {
//         sc2::Point3D p = m_enemy_bases[2].town_hall_location;
//         OverlordObject* o = m_overlords.GetClosestTo(p);
//         o->SetDestination(sc2::Point3D(p.x, p.y, 0.0f));
//         o->SetJob(OverlordJob::SCOUTTHIRD);
//         working_list.Remove(*o);
//     }

    // SCOUTMAP/STANDBY
//     int count = 0;
//     for(OverlordObject& o : working_list())
//     {
//         if (count < 10)
//         {
//             o.SetDestination(o.GetPos());
//             o.SetJob(OverlordJob::SCOUTMAP);
//             working_list.Remove(o);
//         }
//         else
//         {
//             o.SetDestination(gHub->GetExpansions().front().town_hall_location);
//             o.SetJob(OverlordJob::STANDBY);
//             working_list.Remove(o);
//         }
// 
// 
//         ++count;
//     }// 
// }

// ------------------------------------------------------------------
// Cache<OverlordObject&> OverlordFarm::GetOverlordsWithJob(OverlordJob job_ /*= OverlordJob::STANDBY*/)
// {
//     Cache<OverlordObject&> out;
//     for(OverlordObject& o : m_overlords())
//     {
//         if(o.GetJob() == job_)
//         {
//             out.Add(o);
//         }
//     }
// 
//     return out;
// }

// ------------------------------------------------------------------
// Cache<OverlordObject&> OverlordFarm::GetOverlordsWithJob(Cache<OverlordObject&>& overlords, OverlordJob job_ /*= OverlordJob::STANDBY*/)
// {
//     Cache<OverlordObject&> out;
//     for (OverlordObject& o : overlords())
//     {
//         if (o.GetJob() == job_)
//         {
//             out.Add(o);
//         }
//     }
// 
//     return out;
// }
