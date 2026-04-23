#include "stdafx.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "AnimatingMeshObject.h"
#include "MeshAssetManager.h"
#include "GameMain.h"

void GameObject::ConfigureInstance(EntityUid instanceUid, GameObjectController* objectController, GameObjectDefinition* objectDef)
{
    mInstanceUid = instanceUid;

    cxx_assert((mDefinition == nullptr) && objectDef);
    mDefinition = objectDef;

    cxx_assert(mObjectController == nullptr);
    mObjectController = objectController;

    mObjectState = mDefinition->mStartState;

    // controller is optional for simpler objects
    if (mObjectController)
    {
        mObjectController->ConfigureInstance(this);
    }
}

void GameObject::SpawnInstance()
{
    cxx_assert(!mObjectLifecycleFlags.mWasSpawned);
    cxx_assert(!mObjectLifecycleFlags.mWasDespawned);
    cxx_assert(!mObjectLifecycleFlags.mWasDeleted);

    if (mObjectLifecycleFlags.mWasSpawned) return;

    mOwnHandle = GetObjectManager().GetObjectHandle(mInstanceUid);

    CreateMeshObject();

    if (mObjectController)
    {
        mObjectController->SpawnInstance();
    }

    mObjectLifecycleFlags.mWasSpawned = true;
}

void GameObject::DespawnInstance()
{
    cxx_assert(mObjectLifecycleFlags.mWasSpawned);

    if (mObjectLifecycleFlags.mWasDespawned) return;

    if (mObjectController)
    {
        mObjectController->DespawnInstance();
    }

    DisableLocomotion();
    DisablePhysics();
    DestroyMeshObject();

    mOwnHandle = {};

    mObjectLifecycleFlags.mWasDespawned = true;
}

void GameObject::SetObjectPosition(const glm::vec3& position)
{
    mObjectTransform.mPosition = position;
    if (mMeshObject)
    {
        mMeshObject->SetPosition(mObjectTransform.mPosition);
    }
    // cancel movement
    if (mObjectLocomotion)
    {
        mObjectLocomotion->StopMovement();
    }
    // sync physics
    if (mObjectPhysics)
    {
        mObjectPhysics->SetTransform(mObjectTransform);
    }
}

void GameObject::SetObjectPosition(const glm::vec2& position)
{
    const glm::vec3 position3d { position.x, mObjectTransform.mPosition.y, position.y };
    SetObjectPosition(position3d);
}

void GameObject::SnapObjectPositionToFloor(bool withRespectToMeshBounds)
{
    float vertPosition = GetGameWorld().GetGameMap().GetFloorHeightAt(mObjectTransform.mPosition);

    if (withRespectToMeshBounds)
    {
        float extraOffsetFromFloor = 0.0f;
        // get the offset between the y position and the mesh's bottom aabbox
        const cxx::aabbox& meshLocalBounds = GetMeshLocalBounds();
        if (meshLocalBounds.is_valid())
        {
            extraOffsetFromFloor = (0.0f - meshLocalBounds.mMin.y);
        }
        vertPosition += extraOffsetFromFloor;
    }

    if (cxx::eps_equals(vertPosition, mObjectTransform.mPosition.y))
        return;

    mObjectTransform.mPosition.y = vertPosition;
    if (mMeshObject)
    {
        mMeshObject->SetPosition(mObjectTransform.mPosition);
    }
}

void GameObject::SetObjectOrientation(cxx::angle_t orientation)
{
    mObjectTransform.mOrientation = orientation;
    if (mMeshObject)
    {
        mMeshObject->ResetOrientation();
        mMeshObject->RotateAroundAxis(WorldAxes::Y, mObjectTransform.mOrientation);
    }
    // cancel rotation
    if (mObjectLocomotion)
    {
        mObjectLocomotion->StopOrient();
    }
    // sync physics
    if (mObjectPhysics)
    {
        mObjectPhysics->SetTransform(mObjectTransform);
    }
}

const cxx::aabbox& GameObject::GetMeshLocalBounds() const
{
    static const cxx::aabbox nullBounds;
    if (mMeshObject)
        return mMeshObject->GetLocalBounds();

    return nullBounds;
}

const cxx::aabbox& GameObject::GetMeshWorldBounds() const
{
    static const cxx::aabbox nullBounds;
    if (mMeshObject)
    {
        mMeshObject->ComputeTransformation();
        return mMeshObject->GetWorldBounds();
    }
    return nullBounds;
}

MapPoint2D GameObject::GetTilePosition() const
{
    const glm::vec3 position = GetObjectPosition();
    return MapUtils::ComputeTileFromPosition(position);
}

void GameObject::OnRecycle()
{
    DespawnInstance();

    mObjectPhysics = nullptr;
    mObjectController = nullptr;
    mInstanceUid = 0;
    mObjectState = eGameObjectState_None;
    mParentRoom = {};
    mObjectComponents = {};
    mObjectCapabilities = {};
    mDefinition = nullptr;
    mObjectTransform.mPosition = {};
    mObjectTransform.mOrientation = {};
    mResourceMeshId = eGameObjectMeshId_Main;
    mObjectLifecycleFlags = {};
}

void GameObject::CreateMeshObject()
{
    if (mMeshObject != nullptr) return;

    if (!mDefinition->mResourceMesh.IsDefined())
        return;

    mMeshObject = GetScene().CreateAnimatingMesh();
    cxx_assert(mMeshObject);

    if (mMeshObject)
    {
        // sync mesh transformation with object
        mMeshObject->ResetTransformation();
        mMeshObject->SetPosition(mObjectTransform.mPosition);
        mMeshObject->RotateAroundAxis(WorldAxes::Y, mObjectTransform.mOrientation);

        ConfigureMeshObject(mDefinition->mResourceMesh);

        mMeshObject->SetActive(true);    
    }
}

bool GameObject::HasMeshResource(eGameObjectMeshId meshId) const
{
    switch (meshId)
    {
        case eGameObjectMeshId_Main: return mDefinition->mResourceMesh.IsDefined();
        case eGameObjectMeshId_Additional1: return mDefinition->mResourceAdditional1.IsDefined();
        case eGameObjectMeshId_Additional2: return mDefinition->mResourceAdditional2.IsDefined();
        case eGameObjectMeshId_Additional3: return mDefinition->mResourceAdditional3.IsDefined();
        case eGameObjectMeshId_Additional4: return mDefinition->mResourceAdditional4.IsDefined();
    }
    cxx_assert(false);
    return false;
}

bool GameObject::SetMeshResource(eGameObjectMeshId meshId)
{
    if (mResourceMeshId == meshId) return true;

    std::reference_wrapper<ArtResourceDefinition> artResource = mDefinition->mResourceMesh;

    switch (meshId)
    {
        case eGameObjectMeshId_Additional1: 
            artResource = mDefinition->mResourceAdditional1;
        break;
        case eGameObjectMeshId_Additional2: 
            artResource = mDefinition->mResourceAdditional2;
        break;
        case eGameObjectMeshId_Additional3: 
            artResource = mDefinition->mResourceAdditional3;
        break;
        case eGameObjectMeshId_Additional4: 
            artResource = mDefinition->mResourceAdditional4;
        break;
    }

    if (artResource.get().IsDefined())
    {
        mResourceMeshId = meshId;

        CreateMeshObject();
        ConfigureMeshObject(artResource.get());
        return true;
    }
    return false;
}

void GameObject::ConfigureMeshObject(const ArtResourceDefinition& artResource)
{
    if (mMeshObject == nullptr) return;

    bool isSucess = false;

    if ((artResource.mResourceType == eArtResource_Mesh) ||
        (artResource.mResourceType == eArtResource_TerrainMesh))
    {
        MeshAsset* meshAsset = gMeshAssetManager.GetMesh(artResource.mResourceName);
        mMeshObject->Configure(meshAsset);
        isSucess = true;
    }

    if (artResource.mResourceType == eArtResource_AnimatingMesh)
    {
        MeshAsset* meshAsset = gMeshAssetManager.GetMesh(artResource.mResourceName);

        AnimatingMeshObject::AnimationParams animParams;
        animParams.mFramesPerSecond = artResource.mAnimationDesc.mFps * 1.0f;
        animParams.mFirstFrame = 0;
        animParams.mLastFrame = (artResource.mAnimationDesc.mFrames > 0) ? (artResource.mAnimationDesc.mFrames - 1) : 0;
        animParams.mIsLoopEnabled = !artResource.mDoesntLoop;
        mMeshObject->Configure(meshAsset, animParams);
        mMeshObject->ResetAnimationSpeedFactor();
        // set random progress
        if (artResource.mRandomStartFrame)
        {
            mMeshObject->SetAnimationProgress(Random::GenerateFloat01());
        }
        isSucess = true;
    }

    cxx_assert(isSucess);
}

void GameObject::SetObjectState(eGameObjectState stateId)
{
    mObjectState = stateId;
}

void GameObject::SyncWithPhysicsTransform(const EntityTransform& entityTransform)
{
    mObjectTransform = entityTransform;
    if (mMeshObject)
    {
        mMeshObject->SetPosition(mObjectTransform.mPosition);
        mMeshObject->ResetOrientation();
        mMeshObject->RotateAroundAxis(WorldAxes::Y, mObjectTransform.mOrientation);
    }
}

void GameObject::EnableLocomotion()
{
    if (mObjectLocomotion) return;

    GetGameWorld().GetLocomotionService().AttachUser(this);

    mObjectLocomotion = GetGameWorld().GetLocomotionService().GetGameObjectLocomotion(this);
    cxx_assert(mObjectLocomotion);
    mObjectLocomotion->Configure(this);
    mObjectLocomotion->Stop();
}

void GameObject::DisableLocomotion()
{
    if (mObjectLocomotion)
    {
        GetGameWorld().GetLocomotionService().DetachUser(this);
        mObjectLocomotion = nullptr;
    }
}

bool GameObject::RescaleAnimationDuration(float animDuration)
{
    cxx_assert(animDuration > 0.0f);
    if (mMeshObject && mMeshObject->HasAnimation())
    {
        float origDuration = mMeshObject->GetAnimationDuration();
        if (animDuration > 0.0f)
        {
            float speedFactor = origDuration / animDuration;
            cxx_assert(speedFactor > 0.0f);
            mMeshObject->SetAnimationSpeedFactor(speedFactor);
        }
        return true;
    }
    return false;
}

void GameObject::ResetAnimationDuration()
{
    if (mMeshObject && mMeshObject->HasAnimation())
    {
        mMeshObject->ResetAnimationSpeedFactor();
    }
}

void GameObject::ParentRoomChanged(EntityHandle roomHandle)
{
    if (mParentRoom == roomHandle) return;
    if (roomHandle && !roomHandle.IsRoom())
    {
        cxx_assert(false);
        return;
    }
    mParentRoom = roomHandle;
    if (mObjectController)
    {
        mObjectController->ParentRoomChanged();
    }
}

void GameObject::EnablePhysics()
{
    if (mObjectPhysics) return;

    GetGameWorld().GetPhysics().AttachUser(mOwnHandle, mObjectTransform, this);

    mObjectPhysics = GetGameWorld().GetPhysics().GetPhysicsObject(mOwnHandle);
    cxx_assert(mObjectPhysics);
    mObjectPhysics->ClearAngularVelocity();
    mObjectPhysics->ClearLinearVelocity();
}

void GameObject::DisablePhysics()
{
    if (mObjectPhysics)
    {
        GetGameWorld().GetPhysics().DetachUser(mOwnHandle);
        mObjectPhysics = nullptr;
    }
}

void GameObject::DestroyMeshObject()
{
    if (mMeshObject)
    {
        mMeshObject->SetActive(false);
        mMeshObject.reset();
    }
}

void GameObject::SetObjectDeleted()
{
    mObjectLifecycleFlags.mWasDeleted = true;
}
