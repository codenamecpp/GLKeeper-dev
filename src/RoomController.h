#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Room.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class RoomController: public GameSessionAware
{
public:

    //////////////////////////////////////////////////////////////////////////

    // temporary array for room object evaluation

    using FurnitureEvaluationResult = Temp_Vector<RoomFurnitureSlot>;

    //////////////////////////////////////////////////////////////////////////

public:
    virtual ~RoomController();

    // lifecycle

    // performs early initialization of the instance, before it is spawned
    virtual void ConfigureInstance(Room* roomInstance);

    // preloads resources, adds the instance to the game world and activates it
    virtual void SpawnInstance();

    // deactivates the instance and releases its internal resources
    virtual void DespawnInstance();

    // fixed logic tick update
    virtual void UpdateLogicTick(float stepDeltaTime);

    virtual void EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation);
    virtual void EvaluateWallFurniture(FurnitureEvaluationResult& evaluation);
    virtual void EvaluatePillars(FurnitureEvaluationResult& evaluation);
    virtual void PostRearrangeObjects();
    virtual void PostReconfigureRoom();

public:
    // pool
    virtual void OnRecycle();

protected:
    virtual void ReevaluatePillarTiles();

    // shortcuts
    inline Room* GetRoomInstance() const { return mRoomInstance; }
    inline RoomDefinition* GetRoomDefinition() const { return mRoomInstance->GetDefinition(); }
    inline ePlayerID GetRoomOwnerID() const { return mRoomInstance->GetRoomOwnerID(); }
    inline EntityUid GetRoomInstanceUid() const { return mRoomInstance->GetRoomInstanceUid(); }
    inline EntityHandle GetRoomInstanceHandle() const { return mRoomInstance->GetOwnHandle(); }
    inline GameObjectClassId GetRoomPillarObjectId() const
    {
        RoomDefinition* roomDefinition = GetRoomDefinition();
        return roomDefinition->mPillarObjectId;
    }

    // gateway

    inline RoomCapabilities& GetRoomCapabilities() const { return mRoomInstance->mRoomCapabilities; }
    inline RoomComponents& GetRoomComponents() const { return mRoomInstance->mRoomComponents; }

protected:
    Room* mRoomInstance = nullptr;

    std::vector<MapTile*> mPillarTiles; // cache
};
