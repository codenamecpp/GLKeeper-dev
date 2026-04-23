#include "stdafx.h"
#include "GameObjectController.h"

GameObjectController::~GameObjectController()
{

}

void GameObjectController::ConfigureInstance(GameObject* objectInstance)
{
    cxx_assert(mGameObjectInstance == nullptr);
    mGameObjectInstance = objectInstance;
    cxx_assert(mGameObjectInstance);
}

void GameObjectController::SpawnInstance()
{

}

void GameObjectController::DespawnInstance()
{

}

void GameObjectController::UpdateLogicTick(float stepDeltaTime)
{

}

void GameObjectController::ParentRoomChanged()
{

}

void GameObjectController::OnRecycle()
{
    DespawnInstance();
    mGameObjectInstance = nullptr;
}
