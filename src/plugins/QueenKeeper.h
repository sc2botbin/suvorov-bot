#pragma once

#include "Hub.h"
#include "Plugin.h"
#include "objects/QueenObject.h"

struct QueenKeeper : Plugin
{
    QueenKeeper();

    void OnStep(Builder* builder_) final;
    void OnUnitCreated(const sc2::Unit* unit_, Builder*) final;
    void OnUnitDestroyed(const sc2::Unit* unit_, Builder*) final;

 private:
    void RebalanceQueens();
    Cache<QueenObject> m_free_queens;
    Cache<QueenObject> m_hatchery_queens;
};
