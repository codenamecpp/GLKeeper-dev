#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObjectDefs.h"

//////////////////////////////////////////////////////////////////////////

// simple locomotion controller of game object

//////////////////////////////////////////////////////////////////////////

class GameObjectLocomition final: public cxx::noncopyable
{
    friend class LocomotionService;

public:
    GameObjectLocomition() = default;

    void Configure(GameObject* gameObject);

    // reach orientation
    void ReachOrientationToPoint(const glm::vec2& position);
    void ReachOrientation(cxx::angle_t orientation);
    
    void SetAlignTolerance(float tolerance);

    // arrive to destination
    void ArriveTo(const glm::vec2& position);

    void SetArrivalTolerance(float tolerance);

    // interrupting
    void StopMovement();
    void StopOrient();
    void Stop();

    // whether object is moving or rotating
    inline bool IsActive() const { return mIsOrienting || mIsMoving; }
    inline bool IsMoving() const { return mIsMoving; }
    inline bool IsOrienting() const { return mIsOrienting; }

public:
    // pool
    void OnRecycle();

private:
    GameObject* mGameObject = nullptr;

    // params
    float mArriveSpeed = 0.0f; // meters per second
    float mArrivalTolerance = 0.01f;
    float mOrientSpeed = 0.0f; // rads per second
    float mAlignTolerance = 0.01f;

    bool mIsMoving = false;
    bool mIsOrienting = false;

    cxx::angle_t mDesiredOrientation {};
    glm::vec2 mDesiredPosition {};
};