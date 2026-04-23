#include "stdafx.h"
#include "GoldPileObjectController.h"

void GoldPileObjectController::ConfigureInstance(GameObject* objectInstance)
{
    GameObjectController::ConfigureInstance(objectInstance);

    // wire components
    GameObjectComponents& objectComponents = GetObjectComponents();
    if (objectComponents.mGoldContainerData.has_value())
    {
        mGoldComponent = &(*objectComponents.mGoldContainerData);
    }
    cxx_assert(mGoldComponent);

    // setup capabilities
    GameObjectCapabilities& objectCapabilities = GetObjectCapabilities();
    if (mGoldComponent)
    {
        objectCapabilities.mGoldContainer = this;
    }
}

void GoldPileObjectController::SpawnInstance()
{
    GameObjectController::SpawnInstance();
    SetMeshFromGoldAmount();
}

void GoldPileObjectController::DespawnInstance()
{
    GameObjectController::DespawnInstance();
}

void GoldPileObjectController::UpdateLogicTick(float stepDeltaTime)
{
    GameObjectController::UpdateLogicTick(stepDeltaTime);
}

void GoldPileObjectController::OnRecycle()
{
    GameObjectController::OnRecycle();

    mGoldComponent = nullptr;
}

long GoldPileObjectController::GetStoredGoldAmount() const
{
    return mGoldComponent->mGoldAmount;
}

long GoldPileObjectController::GetStoredGoldCapacity() const
{
    return mGoldComponent->mGoldCapacity;
}

long GoldPileObjectController::StoreGold(long goldAmount)
{
    long maxCapacity = GetStoredGoldCapacity();

    long oldGoldAmount = mGoldComponent->mGoldAmount;
    long newGoldAmount = (oldGoldAmount + goldAmount);
    if (maxCapacity > 0)
    {
        newGoldAmount = std::min(maxCapacity, newGoldAmount);
    }
    if (newGoldAmount > oldGoldAmount)
    {
        mGoldComponent->mGoldAmount = newGoldAmount;
        SetMeshFromGoldAmount();
        return newGoldAmount - oldGoldAmount;
    }
    return 0;
}

long GoldPileObjectController::DisposeGold(long goldAmount)
{
    long oldGoldAmount = mGoldComponent->mGoldAmount;
    long newGoldAmount = std::max(0L, oldGoldAmount - goldAmount);
    if (newGoldAmount < oldGoldAmount)
    {
        mGoldComponent->mGoldAmount = newGoldAmount;
        SetMeshFromGoldAmount();
        return oldGoldAmount - newGoldAmount;
    }
    return 0;
}

void GoldPileObjectController::SetMeshFromGoldAmount()
{
    const eGameObjectMeshId animIds[] = {
        eGameObjectMeshId_Additional1,
        eGameObjectMeshId_Additional2,
        eGameObjectMeshId_Main};

    long numStages = CountOf(animIds);

    eGameObjectMeshId meshId = eGameObjectMeshId_Main;

    long goldPerStage = (mGoldComponent->mGoldCapacity / numStages);
    if (goldPerStage > 0)
    {
        long currStage = std::min((mGoldComponent->mGoldAmount / goldPerStage), numStages - 1);
        meshId = animIds[currStage];
    }
    GameObject* objectInstance = GetObjectInstance();
    objectInstance->SetMeshResource(meshId);
}
