#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "ScenarioDefs.h"
#include "Room.h"
#include "RoomController.h"
#include "EntityDefs.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class RoomManager: public GameSessionAware
{
private:

    //////////////////////////////////////////////////////////////////////////
    struct RoomInstanceSlot { uint32_t mGeneration = 1; 
        cxx::uniqueptr<Room> mInstance; 
        cxx::uniqueptr<RoomController> mController;
    };
    //////////////////////////////////////////////////////////////////////////

public:

    bool LoadScenario(const ScenarioDefinition& scenarioDef);
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdateLogicTick(float stepDeltaTime);

    // forcibly refresh active room lists, destroy rooms pending destruction
    // note: this will invalidate list iterators
    void ProcessRoomChanges();

    EntityHandle CreateRoom(RoomTypeId typeId, ePlayerID ownerID);
    EntityHandle CreateRoom(RoomDefinition* roomDefinition, ePlayerID ownerID);

    EntityHandle GetRoomHandle(EntityUid instanceUid) const;

    Room* GetRoomPtr(EntityUid instanceUid) const;
    Room* GetRoomPtr(const EntityHandle& roomHandle) const;

    RoomController* GetRoomController(EntityUid instanceUid) const;
    RoomController* GetRoomController(const EntityHandle& roomHandle) const;

    bool ActivateRoom(const EntityHandle& roomHandle);
    bool ActivateRoom(EntityUid instanceUid);

    bool DeleteRoom(const EntityHandle& roomHandle);
    bool DeleteRoom(EntityUid instanceUid);

    // get all currently active rooms in game world
    // warning: do not store result
    inline cxx::span<Room*> GetActiveRooms() const { return mActiveRooms; }
    inline cxx::span<Room*> GetActiveRoomsByType(RoomTypeId roomTypeId) const
    {
        auto map_it = mActiveRoomsByType.find(roomTypeId);
        if (map_it != mActiveRoomsByType.end())
        {
            return map_it->second;
        }
        return {};
    }

    // get number of currently active rooms in game world
    inline int GetActiveRoomCount() const { return static_cast<int>(mActiveRooms.size()); }

private:
    // factory
    cxx::uniqueptr<Room> GetNewRoomInstance() const;

    template<typename TRoomController>
    cxx::uniqueptr<RoomController> GetNewRoomControllerInstance() const;
    cxx::uniqueptr<RoomController> GetNewRoomControllerInstance(RoomDefinition* roomDefinition) const;

    void RegisterRoomsInRegistrationQueue();
    void DestroyRoomsInRemoveQueue();
    void DestroyRooms();

    void ConfigureNewRoomInstance(Room* roomInstance, 
        RoomController* roomController, 
        RoomDefinition* roomDefinition, EntityUid instanceUid, ePlayerID ownerID);

    void RegisterRoom(Room* roomInstance);
    void UnregisterRoom(Room* roomInstance);

private:
    std::vector<RoomInstanceSlot> mRoomSlots;
    std::unordered_map<EntityUid, EntityHandle> mInstanceUidsMap;
    std::unordered_map<RoomTypeId, std::vector<Room*>> mActiveRoomsByType;

    std::vector<Room*> mActiveRooms;
    std::vector<EntityHandle> mRegistrationQueue; // pending registration in lists
    std::vector<EntityHandle> mRemoveQueue; // pending destroy
};
