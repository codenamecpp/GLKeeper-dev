#include "stdafx.h"
#include "ChickenObjectController.h"
#include "GameWorld.h"
#include "GameMain.h"

void ChickenObjectController::ConfigureInstance(GameObject* objectInstance)
{
    GameObjectController::ConfigureInstance(objectInstance);
}

void ChickenObjectController::SpawnInstance()
{
    GameObjectController::SpawnInstance();

    GetObjectInstance()->EnablePhysics();

    switch (GetObjectClassId())
    {
        case GameObjectClassId_Egg:
            InitEgg();
        break;

        case GameObjectClassId_Chicken:
            InitChicken();
        break;

        default:
            cxx_assert(false);
        break;
    }
}

void ChickenObjectController::DespawnInstance()
{
    GameObjectController::DespawnInstance();
    mTicksBeforeHatch = 0;
    mPeckingChangeDirectionCountdown = 0;
    mPeckingChangeDirectionTicks = 0;
    mLifetimeTicks = 0; // testing
}

void ChickenObjectController::UpdateLogicTick(float stepDeltaTime)
{
    GameObjectController::UpdateLogicTick(stepDeltaTime);

    // egg logic

    if (GetObjectClassId() == GameObjectClassId_Egg)
    {
        eGameObjectState gameObjectState = GetObjectInstance()->GetObjectState();

        if (gameObjectState == eGameObjectState_None)
        {
            if (--mTicksBeforeHatch > 0) return;
            StartHatching();
            return;
        }

        if (gameObjectState == eGameObjectState_Hatching)
        {
            if (--mTicksBeforeHatch > 0) return;

            // spawn chicken
            GetObjectInstance()->SetObjectState(eGameObjectState_None);
            EntityHandle chickenHandle = GetObjectManager().CreateObject(GameObjectClassId_Chicken);
            if (GameObject* chickenObject = GetObjectManager().GetObjectPtr(chickenHandle))
            {
                chickenObject->SetObjectPosition(GetObjectInstance()->GetObjectPosition());
                chickenObject->SetObjectOrientation(GetObjectInstance()->GetObjectOrientation());
                GetObjectManager().ActivateObject(chickenHandle);
            }
            GetObjectManager().DeleteObject(GetObjectInstance()->GetInstanceUid());
            return;
        }

        return;
    }

    // chicken logic

    if (--mLifetimeTicks < 0)
    {
        Die();
        return;
    }

    // wait until locomotion is done
    if (GameObjectLocomition* locomotion = GetObjectLocomotion())
    {
        if (locomotion->IsActive()) return;
    }

    eGameObjectState currentState = GetObjectInstance()->GetObjectState();

    // arrived ?
    if (currentState == eGameObjectState_MoveTo)
    {
        StartPecking();
        return;
    }

    const int PeckingChangeDirectionIntervalTicks = 6; // todo: magic numbers
    const float AngleRangeDegrees = 180.0f;
    if (currentState == eGameObjectState_Pecking)
    {
        ++mPeckingChangeDirectionTicks;
        if (mPeckingChangeDirectionTicks < PeckingChangeDirectionIntervalTicks) return;

        mPeckingChangeDirectionTicks = 0;
        if (--mPeckingChangeDirectionCountdown >= 0)
        {
            cxx::angle_t randomAngle = cxx::angle_t::from_degrees((AngleRangeDegrees * Random::GenerateFloat01()) - (AngleRangeDegrees * 0.5f));
            randomAngle += GetObjectInstance()->GetObjectOrientation();

            if (GameObjectLocomition* locomotion = GetObjectLocomotion())
            {
                locomotion->ReachOrientation(randomAngle);
            }
            return;
        }

        glm::vec2 nextMovePoint {};
        if (!TrySelectRandomMovePoint(nextMovePoint) || !TryStartMoveToNextMovePoint(nextMovePoint))
        {
            StartIdle();
        }
        return;
    }
}

void ChickenObjectController::OnRecycle()
{
    GameObjectController::OnRecycle();
    mPeckingChangeDirectionCountdown = 0;
    mPeckingChangeDirectionTicks = 0;
    mLifetimeTicks = 0;
}

void ChickenObjectController::StartIdle()
{
    GetObjectInstance()->SetObjectState(eGameObjectState_DoNothing);
    GetObjectInstance()->SetMeshResource(eGameObjectMeshId_Additional3);
}

void ChickenObjectController::StartPecking()
{
    GetObjectInstance()->SetObjectState(eGameObjectState_Pecking);
    GetObjectInstance()->SetMeshResource(eGameObjectMeshId_Additional1);
    mPeckingChangeDirectionTicks = 0;
    mPeckingChangeDirectionCountdown = 3; // todo: magic numbers
}

void ChickenObjectController::Die()
{
    mLifetimeTicks = 0;

    GetObjectManager().DeleteObject(GetObjectUid());
}

void ChickenObjectController::StartHatching()
{
    GetObjectInstance()->SetMeshResource(eGameObjectMeshId_Additional1);
    GetObjectInstance()->SetObjectState(eGameObjectState_Hatching);
    GetObjectInstance()->ResetAnimationDuration();

    // reuse tick counter
    mTicksBeforeHatch = 2;  // todo: magic numbers

    // adjust animation duration
    float eggShellDuration = gTime.TicksToSeconds(eFixedClock::GameLogic, mTicksBeforeHatch);
    GetObjectInstance()->RescaleAnimationDuration(eggShellDuration);
}

void ChickenObjectController::InitEgg()
{
    GetObjectInstance()->SetObjectOrientation(Random::GenerateAngle());
    GetObjectInstance()->SetObjectState(eGameObjectState_None);
    GetObjectInstance()->SetMeshResource(eGameObjectMeshId_Main);

    mTicksBeforeHatch = 6; // todo: magic numbers

    // adjust animation duration
    float eggShellDuration = gTime.TicksToSeconds(eFixedClock::GameLogic, mTicksBeforeHatch);
    GetObjectInstance()->RescaleAnimationDuration(eggShellDuration);
}

void ChickenObjectController::InitChicken()
{
    GetObjectInstance()->EnableLocomotion();

    if (GameObjectLocomition* locomotion = GetObjectLocomotion())
    {
        locomotion->Stop();
    }

    mLifetimeTicks = 120;

    glm::vec2 nextMovePoint {};
    if (!TrySelectRandomMovePoint(nextMovePoint) || !TryStartMoveToNextMovePoint(nextMovePoint))
    {
        StartIdle();
    }
}

bool ChickenObjectController::TrySelectRandomMovePoint(glm::vec2& nextMovePoint)
{
    MapTile* tileToGo = nullptr;

    // todo: move to navigation service

    MapPoint2D currentTilePosition = GetObjectInstance()->GetTilePosition();
    if (MapTile* currentTile = GetGameWorld().GetGameMap().GetMapTile(currentTilePosition))
    {
        RoomDefinition* roomDefinition = currentTile->mRoomInstance ? 
            currentTile->mRoomInstance->GetDefinition() : nullptr;

        bool insideHatchery = roomDefinition && (roomDefinition->mRoomType == RoomTypeId_Hatchery);

        // select good tiles 
        Temp_Vector<MapTile*> candidateTiles;
        candidateTiles.reserve(5);
        candidateTiles.push_back(currentTile);
        for (eDirection dir: gStraightDirections)
        {
            MapTile* neighbourTile = currentTile->mNeighbours[dir];
            if (neighbourTile == nullptr) continue;

            if (insideHatchery)
            {
                // prefer hatchery
                if (!currentTile->SameNeighbourRoomInstance(dir)) continue;
            }
            else
            {
                if (neighbourTile->IsBaseTerrainWaterOrLava()) continue;
                if (neighbourTile->IsTerrainSolid()) continue;
            }
            candidateTiles.push_back(neighbourTile);
        }
        int tileIndex = 0;
        if (candidateTiles.size() > 1)
        {
            tileIndex = Random::GenerateInt(0, static_cast<int>(candidateTiles.size()) - 1);
        }
        tileToGo = candidateTiles[tileIndex];
        cxx_assert(tileToGo);
    }

    if (tileToGo)
    {
        glm::vec3 tileCenterPosition = MapUtils::ComputeTileCenter(tileToGo->mTileLocation);
        nextMovePoint = glm::vec2
        {
            tileCenterPosition.x + ((Random::GenerateFloat01() * MAP_TILE_SIZE) - MAP_TILE_HALF_SIZE) * 0.8f,
            tileCenterPosition.z + ((Random::GenerateFloat01() * MAP_TILE_SIZE) - MAP_TILE_HALF_SIZE) * 0.8f,
        };

        return true;
    }

    return false;
}

bool ChickenObjectController::TryStartMoveToNextMovePoint(const glm::vec2& nextMovePoint)
{   
    bool isSuccess = false;
    if (GameObjectLocomition* locomotion = GetObjectLocomotion())
    {
        locomotion->ReachOrientationToPoint(nextMovePoint);
        locomotion->ArriveTo(nextMovePoint);

        GetObjectInstance()->SetObjectState(eGameObjectState_MoveTo);
        GetObjectInstance()->SetMeshResource(eGameObjectMeshId_Main);

        isSuccess = true;
    }

    return isSuccess;
}

