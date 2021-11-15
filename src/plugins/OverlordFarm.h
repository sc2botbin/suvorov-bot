#pragma once

#include "Hub.h"
#include "Plugin.h"
#include "objects/OverlordObject.h"

struct OverlordFarm : Plugin
{
    OverlordFarm();

    void OnGameStart(Builder*) final;
    void OnStep(Builder*) final;
    void OnUnitCreated(const sc2::Unit* unit_, Builder*) final;
    void OnUnitDestroyed(const sc2::Unit* unit_, Builder*) final;

private:
    void Seek(OverlordObject& overlord_, sc2::Point3D& outVelocity_);
    void Avoid(OverlordObject& overlord_, sc2::Point3D& outVelocity_);
    void Wander(OverlordObject& overlord, sc2::Point3D& outVelocity);

    // void UpdateOverlordJobs();
    // Cache<OverlordObject&> GetOverlordsWithJob(OverlordJob job_ = OverlordJob::STANDBY);
    // Cache<OverlordObject&> GetOverlordsWithJob(Cache<OverlordObject&>& overlords, OverlordJob job_ = OverlordJob::STANDBY);

 private:
    Cache<OverlordObject> m_overlords;
    Expansions m_enemy_bases;
};
