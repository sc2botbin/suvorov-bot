#pragma once

#include "GameObject.h"
#include "behavior/BehaviorTree.hpp"
#include "core/Order.h"

enum class OverlordJob
{
    SCOUTMAIN,
    SCOUTNATURAL,
    SCOUTTHIRD,
    SCOUTMAP,
    STANDBY
};

struct OverlordObject : GameObject
{
    explicit OverlordObject(const sc2::Unit& unit_);

    Node::Status RunBehaviorTree();

public:
    inline void SetDestination(sc2::Point3D destination_) { m_destination = destination_; }
    inline void SetVelocity(sc2::Point3D velocity_) { m_velocity = velocity_; }
    inline void SetJob(OverlordJob job_) { m_job = job_; }
    inline void SetWanderAngle(float angle_) { m_wander_angle = angle_; }

    inline sc2::Point3D GetDestination() { return m_destination; }
    inline sc2::Point3D GetVelocity() { return m_velocity; }
    inline float GetMaxSpeed() { return m_max_speed; }
    inline OverlordJob GetJob() { return m_job; }
    inline float GetWanderAngle() { return m_wander_angle; }

    size_t m_frames_to_skip = 5 * 22; // 5 seconds;
    size_t m_skip_until_frame = 0;
    float m_wander_angle_change = 5.0f;

 private:
    BehaviorTree* m_behavior_tree = nullptr;
    sc2::Point3D m_destination;
    sc2::Point3D m_velocity = { 0.0f, 0.0f, 0.0f };
    float m_max_speed = 0.902f + 1.728f;
    float m_wander_angle = 90.0f;

    OverlordJob m_job = OverlordJob::SCOUTMAP;
};
