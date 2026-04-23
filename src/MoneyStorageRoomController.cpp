#include "stdafx.h"
#include "MoneyStorageRoomController.h"
#include "GameObjectManager.h"
#include "EconomyService.h"
#include "MapUtils.h"

//////////////////////////////////////////////////////////////////////////

static const long MoneyStorageDefaultMaxGoldPerTile = 1000;

//////////////////////////////////////////////////////////////////////////

MoneyStorageRoomController::MoneyStorageRoomController()
    : StorageRoomController(1)
    , mMoneyStorageMaxGoldPerTile(MoneyStorageDefaultMaxGoldPerTile)
{
}

void MoneyStorageRoomController::ConfigureInstance(Room* roomInstance)
{
    StorageRoomController::ConfigureInstance(roomInstance);

    // wire components
    RoomComponents& roomComponents = GetRoomComponents();
    if (roomComponents.mGoldStorageData.has_value())
    {
        mGoldComponent = &(*roomComponents.mGoldStorageData);
    }
    cxx_assert(mGoldComponent);

    // setup capabilities
    RoomCapabilities& roomCapabilities = GetRoomCapabilities();
    if (mGoldComponent)
    {
        roomCapabilities.mMoneyStorage = this;
    }
}

void MoneyStorageRoomController::SpawnInstance()
{
    StorageRoomController::SpawnInstance();

    SyncStoredGoldAmount();
}

void MoneyStorageRoomController::DespawnInstance()
{
    StorageRoomController::DespawnInstance();
}

void MoneyStorageRoomController::PostRearrangeObjects()
{
    StorageRoomController::PostRearrangeObjects();
}

void MoneyStorageRoomController::PostReconfigureRoom()
{
    StorageRoomController::PostReconfigureRoom();

    // update max capacity
    if (mGoldComponent)
    {
        mGoldComponent->mGoldCapacity = mMoneyStorageMaxGoldPerTile * GetStorageTilesCount();
    }

    // update stored gold amount
    SyncStoredGoldAmount();

    // auto add loose gold to storage
    ScanForLooseGold();
}

void MoneyStorageRoomController::OnRecycle()
{
    StorageRoomController::OnRecycle();
    mGoldComponent = nullptr;
    mMoneyStorageMaxGoldPerTile = MoneyStorageDefaultMaxGoldPerTile;
}

long MoneyStorageRoomController::GetStoredGoldAmount() const
{
    return mGoldComponent->mGoldAmount;
}

long MoneyStorageRoomController::GetStoredGoldCapacity() const
{
    return mGoldComponent->mGoldCapacity;
}

long MoneyStorageRoomController::StoreGold(long goldAmount)
{
    if ((goldAmount < 1) || (GetFreeStorageSpace() == 0)) 
        return 0;

    long distributedAmount = DistributeGoldBetweenChests(goldAmount, true);
    // update stored gold amount
    if (distributedAmount > 0)
    {
        mGoldComponent->mGoldAmount += distributedAmount;
        GetEconomyService().StoredGoldAmountChanged(GetRoomOwnerID(), GetRoomInstanceHandle(), distributedAmount);
    }
    return distributedAmount;
}

long MoneyStorageRoomController::DisposeGold(long goldAmount)
{
    if ((goldAmount < 0) || (GetStoredGoldAmount() == 0)) 
        return 0;

    Temp_List<EntityHandle> releaseObjects;

    long resultAmount = 0;
    // dispose gold from chests
    for (const RoomStorageSlot& storageSlot: GetRoomInstance()->GetStorageSlots())
    {
        GameObject* objectInstance = GetObjectManager().GetObjectPtr(storageSlot.mObjectHandle);
        cxx_assert(objectInstance);
        if (objectInstance == nullptr) continue;

        auto* storageCapability = objectInstance->GetGoldContainerCapability();
        if (storageCapability == nullptr) continue;

        // trying use chest
        long removedFromChest = storageCapability->DisposeGold(goldAmount);
        if (removedFromChest > 0)
        {
            resultAmount += removedFromChest;
            goldAmount -= removedFromChest;
        }
        // release chest
        if (storageCapability->GetStoredGoldAmount() == 0)
        {
            releaseObjects.push_back(storageSlot.mObjectHandle);
        }
        // done?
        if (goldAmount <= 0) break;
    }

    // release objects
    for (const EntityHandle& roller: releaseObjects)
    {
        if (!UnassignObjectFromStorage(roller))
        {
            cxx_assert(false);
        }
        GetObjectManager().DeleteObject(roller);
    }

    // update stored gold amount
    if (resultAmount > 0)
    {
        mGoldComponent->mGoldAmount -= resultAmount;
        cxx_assert(mGoldComponent->mGoldAmount >= 0);
        GetEconomyService().StoredGoldAmountChanged(GetRoomOwnerID(), GetRoomInstanceHandle(), -resultAmount);
    }
    return resultAmount;
}

long MoneyStorageRoomController::GetFreeStorageSpace() const
{
    long freeCapacity = std::max(mGoldComponent->mGoldCapacity - mGoldComponent->mGoldAmount, 0L);
    return freeCapacity;
}

void MoneyStorageRoomController::StoredObjectUnassigned(EntityHandle entityHandle)
{  
    // delete chest and create gold pile

    GameObject* gameObject = GetObjectManager().GetObjectPtr(entityHandle);
    if (gameObject == nullptr) return;

    if (gameObject->GetObjectClassId() == GameObjectClassId_GoldPile) 
        return;

    if (auto* goldContainer = gameObject->GetGoldContainerCapability())
    {
        long goldAmount = goldContainer->GetStoredGoldAmount();
        if (goldAmount > 0)
        {
            EntityHandle newEntity = GetObjectManager().CreateGoldPile(goldAmount);

            SetObjectPlacementToTileCenter(newEntity, gameObject->GetTilePosition(), true);
            if (!GetObjectManager().ActivateObject(newEntity))
            {
                cxx_assert(false);
            }
        }
    }
    GetObjectManager().DeleteObject(entityHandle);
}

void MoneyStorageRoomController::StoredObjectReassigned(EntityHandle entityHandle, const RoomStorageTile& newStorageTile)
{
    SetObjectPlacementToTileCenter(entityHandle, newStorageTile.mTileLocation, false);
}

void MoneyStorageRoomController::StoredObjectAssigned(EntityHandle entityHandle, const RoomStorageTile& newStorageTile)
{
    SetObjectPlacementToTileCenter(entityHandle, newStorageTile.mTileLocation, true);
}

void MoneyStorageRoomController::SetMoneyStorageMaxGoldPerTile(long maxGoldPerTile)
{
    mMoneyStorageMaxGoldPerTile = maxGoldPerTile;
    cxx_assert(mMoneyStorageMaxGoldPerTile > 0);
}

void MoneyStorageRoomController::SetObjectPlacementToTileCenter(EntityHandle entityHandle, const MapPoint2D& tileLocation, bool setRandomOrientation)
{
    GameObject* gameObject = GetObjectManager().GetObjectPtr(entityHandle);
    cxx_assert(gameObject);

    if (gameObject == nullptr) return;

    glm::vec3 objectPosition = MapUtils::ComputeTileCenter(tileLocation);
    gameObject->SetObjectPosition(objectPosition);
    if (setRandomOrientation)
    {
        gameObject->SetObjectOrientation(Random::GenerateAngle());
    }
    gameObject->SnapObjectPositionToFloor();
}

void MoneyStorageRoomController::SyncStoredGoldAmount()
{
    if (mGoldComponent == nullptr) return;

    long currentGoldAmount = 0;
    GameObjectManager& gobjects = GetObjectManager();
    for (const RoomStorageSlot& storageSlot: GetRoomInstance()->GetStorageSlots())
    {
        GameObject* gameObject = gobjects.GetObjectPtr(storageSlot.mObjectHandle);
        cxx_assert(gameObject);

        if (gameObject == nullptr) continue;

        if (auto* goldContainer = gameObject->GetGoldContainerCapability())
        {
            currentGoldAmount += goldContainer->GetStoredGoldAmount();
        }
    }

    if (mGoldComponent->mGoldAmount != currentGoldAmount)
    {
        long deltaAmount = currentGoldAmount - mGoldComponent->mGoldAmount;
        mGoldComponent->mGoldAmount = currentGoldAmount;
        GetEconomyService().StoredGoldAmountChanged(GetRoomOwnerID(), GetRoomInstanceHandle(), deltaAmount);
    }
}

void MoneyStorageRoomController::ScanForLooseGold()
{
    if ((GetStorageTilesCount() == 0) || (GetFreeStorageSpace() == 0)) 
        return;

    long addedLooseGoldAmount = 0;

    for (GameObject* object: GetObjectManager().GetActiveGoldContainers())
    {
        // ignore deleted
        if (object->IsObjectDeleted()) continue;

        // ingore owned
        if (object->GetParentRoom().IsRoom()) continue;

        // whether object is on one of storage tiles
        MapPoint2D mapLocation = object->GetTilePosition();

        RoomStorageTile* storageTile = GetRoomStorageTileFromLocation(mapLocation);
        if (storageTile == nullptr) continue;

        auto* goldContainer = object->GetGoldContainerCapability();
        cxx_assert(goldContainer);
        
        // try store on target tile first
        long storedAmount = StoreGoldOnStorageTile(*storageTile, goldContainer->GetStoredGoldAmount(), true);
        if (storedAmount > 0)
        {
            goldContainer->DisposeGold(storedAmount);
            addedLooseGoldAmount += storedAmount;
        }
        // store on any tile
        if (goldContainer->GetStoredGoldAmount() > 0)
        {
            long distributedAmount = DistributeGoldBetweenChests(goldContainer->GetStoredGoldAmount(), true);
            if (distributedAmount > 0)
            {
                goldContainer->DisposeGold(distributedAmount);
                addedLooseGoldAmount += distributedAmount;
            }
        }

        // delete empty containers
        if (goldContainer->GetStoredGoldAmount() == 0)
        {
            GetObjectManager().DeleteObject(object->GetOwnHandle());
        }

        // done?
        if (GetFreeStorageSpace() == 0) break;
    }

    // update stored gold amount
    if (addedLooseGoldAmount > 0)
    {
        mGoldComponent->mGoldAmount += addedLooseGoldAmount;
        GetEconomyService().StoredGoldAmountChanged(GetRoomOwnerID(), GetRoomInstanceHandle(), addedLooseGoldAmount);
    }
}

long MoneyStorageRoomController::StoreGoldOnStorageTile(const MapPoint2D& tileLocation, long goldAmount, bool canCreateChest)
{
    long resultAmount = 0;
    if (RoomStorageTile* storageTile = GetRoomStorageTileFromLocation(tileLocation))
    {
        resultAmount = StoreGoldOnStorageTile(*storageTile, goldAmount, canCreateChest);
    }
    return resultAmount;
}

long MoneyStorageRoomController::StoreGoldOnStorageTile(RoomStorageTile& storageTile, long goldAmount, bool canCreateChest)
{
    long resultAmount = 0;

    cxx_assert(goldAmount > 0);
    if (goldAmount > 0)
    {
        // distribute gold between existing chests
        for (const EntityHandle entHandle: storageTile.mObjects)
        {
            long storedAmount = StoreGoldInContainer(entHandle, goldAmount);
            if (storedAmount > 0)
            {
                resultAmount += storedAmount;
                goldAmount -= storedAmount;
            }
            // done?
            if (goldAmount <= 0) break;
        }

        // create additional chest if needed
        if (canCreateChest)
        {
            while (goldAmount > 0)
            {
                // check free space on storage tile
                bool hasFreeStorageSpace = storageTile.mObjects.size() < GetMaxObjectsPerStorageTile();
                if (!hasFreeStorageSpace) break;

                long storedAmount = std::min(mMoneyStorageMaxGoldPerTile, goldAmount);
                EntityHandle chestObject = GetObjectManager().CreateGoldChest(storedAmount, mMoneyStorageMaxGoldPerTile);
                GetObjectManager().ActivateObject(chestObject);
                if (!chestObject || !AssignObjectToStorageTile(chestObject, storageTile))
                {
                    cxx_assert(false);
                    break;
                }

                SetObjectPlacementToTileCenter(chestObject, storageTile.mTileLocation, true);

                resultAmount += storedAmount;
                goldAmount -= storedAmount;
            }
        }
    }
    return resultAmount;
}

long MoneyStorageRoomController::StoreGoldInContainer(EntityHandle objectHandle, long goldAmount)
{
    GameObject* objectInstance = GetObjectManager().GetObjectPtr(objectHandle);
    cxx_assert(objectInstance);
    if (objectInstance)
    {
        if (auto* storageCapability = objectInstance->GetGoldContainerCapability())
        {
            return storageCapability->StoreGold(goldAmount);
        }
    }
    return 0;
}

long MoneyStorageRoomController::DistributeGoldBetweenChests(long goldAmount, bool canCreateAdditionalChests)
{
    long resultAmount = 0;

    // distribute gold between existing chests
    for (const RoomStorageSlot& storageSlot: GetRoomInstance()->GetStorageSlots())
    {
        long storedAmount = StoreGoldInContainer(storageSlot.mObjectHandle, goldAmount);
        if (storedAmount > 0)
        {
            resultAmount += storedAmount;
            goldAmount -= storedAmount;
        }
        // done?
        if (goldAmount <= 0) break;
    }

    // create additional chests if needed
    if (canCreateAdditionalChests)
    {
        while (goldAmount > 0)
        {
            RoomStorageTile* storageTile = GetFirstAvailableStorageTile();
            if (storageTile == nullptr) break; // no free storage space

            long storedAmount = StoreGoldOnStorageTile(*storageTile, goldAmount, canCreateAdditionalChests);
            if (storedAmount > 0)
            {
                resultAmount += storedAmount;
                goldAmount -= storedAmount;
            }
        }
    }
    return resultAmount;
}

