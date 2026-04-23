#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObjectDefs.h"
#include "GameObjectLocomition.h"
#include "PhysicsObject.h"

//////////////////////////////////////////////////////////////////////////

class LocomotionService: public cxx::noncopyable
{
public:
    void EnterWorld();
    void ClearWorld();
    void UpdateFrame(float deltaTime);
    void UpdatePhysicsTick(float stepDeltaTime);

    void AttachUser(GameObject* gameObject);
    void DetachUser(GameObject* gameObject);

    GameObjectLocomition* GetGameObjectLocomotion(GameObject* gameObject) const;

private:

    // factory
    cxx::uniqueptr<GameObjectLocomition> CreateGameObjectLocomotion();

    void ProcessLocomotion(GameObjectLocomition* locomotion, PhysicsObject* physics);

private:

    float mSimulationStepDelta = 0.0f;

    //////////////////////////////////////////////////////////////////////////

    struct GameObjectEntry
    {
        GameObject* mGamObject = nullptr;
        cxx::uniqueptr<GameObjectLocomition> mLocomotionState;
    };

    //////////////////////////////////////////////////////////////////////////

    std::vector<GameObjectEntry> mGameObjectEntries;
};