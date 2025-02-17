// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#include "Helpers.h"
#include "Hub.h"
#include "Historican.h"
#include "Map.h"
#include "core/API.h"

#include "sc2lib/sc2_search.h"

#include <cmath>

// ------------------------------------------------------------------
// ------------------------------------------------------------------
namespace
{
Historican gHistory("map");

const float PI = 3.1415927f;

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

size_t CalculateQueries(float radius, float step_size, const sc2::Point2D& center, std::vector<sc2::QueryInterface::PlacementQuery>* queries)
{
    sc2::Point2D current_grid, previous_grid( std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    size_t valid_queries = 0;

    // Find a buildable location on the circumference of the sphere
    float loc = 0.0f;
    while (loc < 360.0f)
    {
        sc2::Point2D point((radius * std::cos((loc * PI) / 180.0f)) + center.x, (radius * std::sin((loc * PI) / 180.0f)) + center.y);

        sc2::QueryInterface::PlacementQuery query(sc2::ABILITY_ID::BUILD_COMMANDCENTER, point);

        current_grid = sc2::Point2D(std::floor(point.x), std::floor(point.y));

        if (previous_grid != current_grid)
        {
            queries->push_back(query);
            ++valid_queries;
        }

        previous_grid = current_grid;
        loc += step_size;
    }

    return valid_queries;
}

// ------------------------------------------------------------------
// ------------------------------------------------------------------
struct Cluster
{
    explicit Cluster(uint64_t id_);

    void AddPoint(const sc2::Point3D& point_);

    float Height() const;

    sc2::Point3D Center() const;

    uint64_t id;
    sc2::Point3D center_of_mass;
    sc2::Point3D town_hall_location;
    std::vector<sc2::Point3D> points;
};

typedef std::vector<Cluster> Clusters;

// ------------------------------------------------------------------
Cluster::Cluster(uint64_t id_): id(id_)
{
    points.reserve(10);
}

// ------------------------------------------------------------------
void Cluster::AddPoint(const sc2::Point3D& point_)
{
    if (points.empty())
    {
        center_of_mass = point_;
    }
    else
    {
        center_of_mass = (center_of_mass * static_cast<float>(points.size() - 1) + point_) / static_cast<float>(points.size());
    }

    points.push_back(point_);
}

// ------------------------------------------------------------------
float Cluster::Height() const
{
    if (points.empty())
        return 0.0f;

    return points.back().z;
}

// ------------------------------------------------------------------
sc2::Point3D Cluster::Center() const
{
    return sc2::Point3D(center_of_mass.x, center_of_mass.y, Height());
}

}  // namespace

// ------------------------------------------------------------------
// ------------------------------------------------------------------
Expansion::Expansion(const sc2::Point3D& town_hall_location_):
    town_hall_location(town_hall_location_), owner(Owner::NEUTRAL),
    town_hall_tag(sc2::NullTag), worker_tag(sc2::NullTag) {
}

// ------------------------------------------------------------------
void Expansion::SetOwner(const sc2::Unit& unit_, Owner owner_) {
    town_hall_tag = unit_.tag;
    owner = owner_;

    // Terran worker_tag should remain as it constructs TownHall
    if (gAPI->observer().GetCurrentRace() != sc2::Race::Terran)
        worker_tag = sc2::NullTag;
}

// ------------------------------------------------------------------
void Expansion::RemoveOwner() {
    owner = Owner::NEUTRAL;
    town_hall_tag = sc2::NullTag;
    worker_tag = sc2::NullTag;
}

// ------------------------------------------------------------------
Expansions CalculateExpansionLocations()
{
    Clusters clusters;
    clusters.reserve(20);

    Units resources = gAPI->observer().GetUnits(IsFoggyResource(), sc2::Unit::Alliance::Neutral);

    if (resources.Empty())
    {
        gHistory.warning() << "No expansions locations could be found!\n";
        return Expansions();
    }

    for (const sc2::Unit* resource : resources())
    {
        bool cluster_found = false;

        for (Cluster& cluster : clusters)
        {
            if (sc2::DistanceSquared3D(resource->pos, cluster.points.back()) < 225.0f)
            {
                cluster.AddPoint(resource->pos);
                cluster_found = true;
                break;
            }
        }

        if (!cluster_found)
        {
            clusters.emplace_back(clusters.size());
            clusters.back().AddPoint(resource->pos);
        }
    }

    std::vector<size_t> query_size;
    std::vector<sc2::QueryInterface::PlacementQuery> queries;
    for (Cluster& cluster : clusters)
    {
        query_size.push_back(CalculateQueries(5.3f, 0.5f, cluster.Center(), &queries));
    }

    std::vector<bool> results = gAPI->query().CanBePlaced(queries);

    size_t start_index = 0;
    Expansions expansions;
    for (Cluster& cluster : clusters)
    {
        for (size_t j = start_index, e = start_index + query_size[cluster.id]; j < e; ++j)
        {
            if (!results[j])
                continue;

            sc2::Point3D town_hall_location = sc2::Point3D(
                queries[j].target_pos.x,
                queries[j].target_pos.y,
                cluster.Height());
            expansions.emplace_back(town_hall_location);
            break;
        }

        start_index += query_size[cluster.id];
    }

    // Include start location. TownHall tag will be added during its OnCreated event.
    expansions.emplace_back(gAPI->observer().StartingLocation());

    return expansions;
}

Expansion GetRandomExpansion()
{
    Expansions e = gHub->GetExpansions();
    int min = 0;
    int max = (int)e.size() - 1;

    static bool first = true;
    if (first)
    {
        srand((unsigned int)time(NULL)); //seeding for the first time only!
        first = false;
    }
    int i = min + rand() % ((max + 1) - min);
    return e[i];
}
