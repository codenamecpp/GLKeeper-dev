#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "GameObjectDefs.h"
#include "ScenarioDefs.h"
#include "AnimatingMeshObject.h"
#include "GameObjectLocomition.h"
#include "EntityDefs.h"
#include "PhysicsDefs.h"
#include "GameSessionAware.h"
#include "GameObjectComponents.h"
#include "GameObjectCapabilities.h"

//////////////////////////////////////////////////////////////////////////

// Defines generic object in dungeon such as room furniture, pillar, crate, gold etc
class GameObject final: public GameSessionAware
    , private PhysicsTransformListener
{
    friend class GameObjectController;
    friend class GameObjectManager;

public:
    GameObject() = default;

    // lifecycle

    // performs early initialization of the object, before it is spawned
    // called by GameObjectManager during CreateObject()
    void ConfigureInstance(EntityUid instanceUid, GameObjectController* objectController, GameObjectDefinition* objectDef);

    // preloads resources, adds the object to the game world and activates it
    // called by GameObjectManager during ActivateObject()
    void SpawnInstance();

    // deactivates the object and releases its internal resources
    // called by GameObjectManager during DeleteObject()
    void DespawnInstance();

    // check whether object marked as pending deletion
    inline bool IsObjectDeleted() const { return mObjectLifecycleFlags.mWasDeleted; }

    // position and orientation
    // note: setting position / orientation will cancel current locomotion
    void SetObjectPosition(const glm::vec3& position);
    void SetObjectPosition(const glm::vec2& position);
    inline glm::vec3 GetObjectPosition() const { return mObjectTransform.mPosition; }
    inline glm::vec2 GetObjectPosition2d() const
    {
        return glm::vec2{mObjectTransform.mPosition.x, mObjectTransform.mPosition.z};
    }
    void SnapObjectPositionToFloor(bool withRespectToMeshBounds = false);

    void SetObjectOrientation(cxx::angle_t orientation);
    inline cxx::angle_t GetObjectOrientation() const { return mObjectTransform.mOrientation; }

    // accessing current object transformation
    const EntityTransform& GetObjectTransform() const { return mObjectTransform; }

    MapPoint2D GetTilePosition() const;

    // scene object shortcuts
    const cxx::aabbox& GetMeshLocalBounds() const;
    const cxx::aabbox& GetMeshWorldBounds() const;

    inline GameObjectDefinition* GetDefinition() const { return mDefinition; }
    inline GameObjectController* GetObjectController() const { return mObjectController; }
    inline GameObjectLocomition* GetObjectLocomotion() const { return mObjectLocomotion; }
    inline AnimatingMeshObject* GetMeshObject() const { return mMeshObject.get(); }
    inline PhysicsObject* GetObjectPhysics() const { return mObjectPhysics; }

    // definition shortcuts
    inline GameObjectClassId GetObjectClassId() const
    {
        return mDefinition->mObjectClass;
    }
    inline eGameObjectCategory GetObjectCategory() const
    {
        return mDefinition->mObjectCategory;
    }

    inline EntityHandle GetParentRoom() const { return mParentRoom; }
    inline EntityHandle GetOwnHandle() const { return mOwnHandle; }
    // object instance unique identifier
    inline EntityUid GetInstanceUid() const { return mInstanceUid; }

    // change object mesh 
    bool HasMeshResource(eGameObjectMeshId meshId) const;
    bool SetMeshResource(eGameObjectMeshId meshId);

    // mesh animation
    bool RescaleAnimationDuration(float animDuration);
    void ResetAnimationDuration();

    // state
    void SetObjectState(eGameObjectState stateId);
    inline eGameObjectState GetObjectState() const { return mObjectState; }

    // accessing object capabilities
    const GameObjectCapabilities& GetCapabilities() const { return mObjectCapabilities; }

    inline GameObjectCapabilities::GoldContainer* GetGoldContainerCapability() const 
    { 
        return mObjectCapabilities.mGoldContainer; 
    }

public:
    // locomotion control
    void EnableLocomotion();
    void DisableLocomotion();

    // physics control
    void EnablePhysics();
    void DisablePhysics();

public:
    // pool
    void OnRecycle();

public:
    // notifications
    void ParentRoomChanged(EntityHandle roomHandle);

private:
    // override PhysicsTransformListener
    void SyncWithPhysicsTransform(const EntityTransform& entityTransform) override;

private:
    // mark object as pending deletion
    void SetObjectDeleted();

    void CreateMeshObject();
    void DestroyMeshObject();
    void ConfigureMeshObject(const ArtResourceDefinition& artResource);

private:
    EntityLifecycleFlags mObjectLifecycleFlags {};

    GameObjectDefinition* mDefinition = nullptr; // never changes

    EntityHandle mOwnHandle;
    // the room where the object is stored, optional
    EntityHandle mParentRoom;

    EntityUid mInstanceUid = 0;

    eGameObjectState mObjectState = eGameObjectState_None;

    GameObjectController* mObjectController = nullptr; // optional
    GameObjectLocomition* mObjectLocomotion = nullptr; // optional
    PhysicsObject* mObjectPhysics = nullptr; // optional

    cxx::uniqueptr<AnimatingMeshObject> mMeshObject; // optional
    eGameObjectMeshId mResourceMeshId = eGameObjectMeshId_Main;

    EntityTransform mObjectTransform;

    GameObjectComponents mObjectComponents;
    GameObjectCapabilities mObjectCapabilities;
};