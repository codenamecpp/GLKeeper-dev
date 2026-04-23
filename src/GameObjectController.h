#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObject.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class GameObjectController: public GameSessionAware
{
public:
    GameObjectController() = default;
    virtual ~GameObjectController();

    // performs early initialization of the instance, before it is spawned
    virtual void ConfigureInstance(GameObject* objectInstance);

    // preloads resources, adds the instance to the game world and activates it
    virtual void SpawnInstance();

    // deactivates the instance and releases its internal resources
    virtual void DespawnInstance();

    // fixed logic tick update
    virtual void UpdateLogicTick(float stepDeltaTime);

    virtual void ParentRoomChanged();

public:
    // pool
    virtual void OnRecycle();

protected:

    // shortcuts

    inline GameObject* GetObjectInstance() const { return mGameObjectInstance; }
    inline GameObjectDefinition* GetObjectDefinition() const
    {
        return mGameObjectInstance->GetDefinition();
    }
    inline GameObjectLocomition* GetObjectLocomotion() const
    {
        return mGameObjectInstance->GetObjectLocomotion();
    }
    inline GameObjectClassId GetObjectClassId() const
    {
        return mGameObjectInstance->GetObjectClassId();
    }
    inline EntityUid GetObjectUid() const
    {
        return mGameObjectInstance->GetInstanceUid();
    }

    // gateway

    inline GameObjectCapabilities& GetObjectCapabilities() const
    {
        return mGameObjectInstance->mObjectCapabilities;
    }
    inline GameObjectComponents& GetObjectComponents() const
    {
        return mGameObjectInstance->mObjectComponents;
    }

protected:
    GameObject* mGameObjectInstance = nullptr;
};