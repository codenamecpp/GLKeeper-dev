#include "stdafx.h"
#include "GameObjectLocomition.h"
#include "GameObject.h"

void GameObjectLocomition::Configure(GameObject* gameObject)
{
    Stop();

    mGameObject = gameObject;
    cxx_assert(mGameObject);

    // configure speeds
    if (mGameObject)
    {
        GameObjectDefinition* definition = mGameObject->GetDefinition();

        mArriveSpeed = definition->mSpeed * MAP_TILE_SIZE;
        cxx_assert(mArriveSpeed >= 0.0f);

        mOrientSpeed = glm::radians(210.0f); // todo: magic values
        cxx_assert(mOrientSpeed >= 0.0f);
    }
}

void GameObjectLocomition::StopOrient()
{
    if (mIsOrienting)
    {
        mDesiredOrientation = {};
        mIsOrienting = false;
    }
}

void GameObjectLocomition::StopMovement()
{
    if (mIsMoving)
    {
        mDesiredPosition = {};
        mIsMoving = false;
    }
}

void GameObjectLocomition::OnRecycle()
{
    Stop();

    mArriveSpeed = 0.0f;
    mOrientSpeed = 0.0f;
    mArrivalTolerance = 0.01f;
    mAlignTolerance = 0.01f;

    mGameObject = nullptr;
}

void GameObjectLocomition::SetArrivalTolerance(float tolerance)
{
    mArrivalTolerance = tolerance;
}

void GameObjectLocomition::SetAlignTolerance(float tolerance)
{
    mAlignTolerance = tolerance;
}

void GameObjectLocomition::ArriveTo(const glm::vec2& position)
{
    StopMovement();

    mDesiredPosition = position;
    mIsMoving = true;
}

void GameObjectLocomition::ReachOrientation(cxx::angle_t orientation)
{
    cxx_assert(mGameObject);

    StopOrient();

    mDesiredOrientation = orientation;
    mIsOrienting = true;
}

void GameObjectLocomition::ReachOrientationToPoint(const glm::vec2& position)
{
    cxx_assert(mGameObject);

    StopOrient();

    const glm::vec2 directionToTarget = glm::normalize(position - mGameObject->GetObjectPosition2d());

    // note:
    // standard orientation (0 rad) is positive X, whereas game objects' normal orientation is negative Y

    mDesiredOrientation = cxx::angle_t::from_radians(::atan2f(directionToTarget.x, directionToTarget.y));
    mIsOrienting = true;
}

void GameObjectLocomition::Stop()
{
    StopMovement();
    StopOrient();
}
