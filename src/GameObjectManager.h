#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObject.h"
#include "ScenarioDefs.h"
#include "GameObjectController.h"
#include "EntityDefs.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class GameObjectManager: public GameSessionAware
{
private:
    
    //////////////////////////////////////////////////////////////////////////
    struct GameObjectSlot { uint32_t mGeneration = 1; 
        cxx::uniqueptr<GameObject> mObject; 
        cxx::uniqueptr<GameObjectController> mController;
    };
    //////////////////////////////////////////////////////////////////////////

public:

    bool LoadScenario(const ScenarioDefinition& scenarioDef);
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdateLogicTick(float stepDeltaTime);

    // forcibly refresh active game object lists, destroy objects pending destruction
    // note: this will invalidate list iterators
    void ProcessObjectChanges();

    EntityHandle CreateScenarioObject(const ScenarioObjectThing& objectThing);
    EntityHandle CreateObject(GameObjectClassId classId);
    EntityHandle CreateObject(GameObjectDefinition* classDefinition);
    EntityHandle CreateGoldChest(long startGold, long storageCapacity);
    EntityHandle CreateGoldPile(long goldAmount);

    EntityHandle GetObjectHandle(EntityUid objectUid) const;

    GameObject* GetObjectPtr(EntityUid objectUid) const;
    GameObject* GetObjectPtr(const EntityHandle& objectHandle) const;

    GameObjectController* GetObjectController(EntityUid instanceUid) const;
    GameObjectController* GetObjectController(const EntityHandle& objectHandle) const;

    bool ActivateObject(const EntityHandle& objectHandle);
    bool ActivateObject(EntityUid objectUid);

    bool DeleteObject(const EntityHandle& objectHandle);
    bool DeleteObject(EntityUid objectUid);

    // get all currently active gameobjects in game world
    // warning: do not store result
    inline cxx::span<GameObject*> GetActiveObjects() const { return mActiveObjects; }
    inline cxx::span<GameObject*> GetActiveGoldContainers() const { return mActiveGoldContainers; }

    cxx::span<GameObject*> GetActiveObjectsByClass(GameObjectClassId classId) const;
    cxx::span<GameObject*> GetActiveObjectsByCategory(eGameObjectCategory objectCategory) const;

private:
    // factory
    cxx::uniqueptr<GameObject> GetNewObjectInstance() const;

    template<typename TController>
    cxx::uniqueptr<GameObjectController> GetNewObjectControllerInstance() const;
    cxx::uniqueptr<GameObjectController> GetNewObjectControllerInstance(GameObjectDefinition* objectDefinition) const;

    void RegisterObjectsInRegistrationQueue();
    void DestroyObjectsInRemoveQueue();
    void DestroyObjects();

    void ConfigureNewObjectInstance(GameObject* gameObject, 
        GameObjectController* objectController, 
        GameObjectDefinition* objectDefinition, EntityUid instanceUid);

    void RegisterObject(GameObject* objectInstance);
    void UnregisterObject(GameObject* objectInstance);

private:
    using GameObjectsList = std::vector<GameObject*>;
    std::vector<GameObjectSlot> mObjectSlots;
    std::unordered_map<EntityUid, EntityHandle> mObjectUidsMap;
    std::unordered_map<GameObjectClassId, GameObjectsList> mActiveObjectsByClass;
    std::unordered_map<eGameObjectCategory, GameObjectsList> mActiveObjectsByCategory;
    GameObjectsList mActiveGoldContainers;
    GameObjectsList mActiveObjects;
    std::vector<EntityHandle> mRegistrationQueue; // pending registration in lists
    std::vector<EntityHandle> mRemoveQueue; // pending destroy
};