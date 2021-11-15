// The MIT License (MIT)
//
// Copyright (c) 2017-2021 Alexander Kurbatov

#pragma once

#include <sc2api/sc2_unit.h>

struct Units
{
    explicit Units(const sc2::Units& units_);

    bool Empty() const;

    const sc2::Units& operator()() const;
    size_t Size() const;

    const sc2::Unit* GetClosestUnit(const sc2::Point2D& point_, float min_ = std::numeric_limits<float>::max()) const;
    const sc2::Unit* GetClosestUnit(sc2::Tag tag_, float min_ = std::numeric_limits<float>::max()) const;
    const sc2::Unit* GetRandomUnit() const;
    sc2::Point3D GetCenter() const;

    bool HasOrder(sc2::ABILITY_ID id_) const;

 private:
    sc2::Units m_units;
};
