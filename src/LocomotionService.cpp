#include "stdafx.h"
#include "LocomotionService.h"
#include "SimplePool.h"
#include "GameObject.h"
#include "TimeManager.h"
#include "GameWorld.h"

void LocomotionService::EnterWorld()
{
    mSimulationStepDelta = gTime.GetFixedDelta(eFixedClock::GamePhysics);
    cxx_assert(mSimulationStepDelta > 0.0f);

    mGameObjectEntries.reserve(1024);
}

void LocomotionService::ClearWorld()
{
    mGameObjectEntries.clear();
}

void LocomotionService::UpdateFrame(float deltaTime)
{
}

void LocomotionService::UpdatePhysicsTick(float stepDeltaTime)
{
    if (mGameObjectEntries.empty())
        return;

    for (const GameObjectEntry& roller: mGameObjectEntries)
    {
        GameObjectLocomition* locomotion = roller.mLocomotionState.get();
        if (!locomotion->IsActive()) 
            continue;

        if (PhysicsObject* physics = roller.mGamObject->GetObjectPhysics())
        {
            ProcessLocomotion(locomotion, physics);
        }
    }
}

void LocomotionService::AttachUser(GameObject* gameObject)
{
    cxx_assert(gameObject);
    if (GetGameObjectLocomotion(gameObject) != nullptr)
    {
        cxx_assert(false);
        return;
    }
    
    GameObjectEntry& objectEntry = mGameObjectEntries.emplace_back();
    objectEntry.mGamObject = gameObject;
    objectEntry.mLocomotionState = CreateGameObjectLocomotion();
}

void LocomotionService::DetachUser(GameObject* gameObject)
{
    if (!cxx::erase_if(mGameObjectEntries, [gameObject](const GameObjectEntry& entry) 
        { 
            return entry.mGamObject == gameObject; 
        }))
    {
        cxx_assert(false);
    }
}

GameObjectLocomition* LocomotionService::GetGameObjectLocomotion(GameObject* gameObject) const
{
    auto object_iter = std::find_if(mGameObjectEntries.begin(), mGameObjectEntries.end(), [gameObject](const GameObjectEntry& entry)
        {
            return entry.mGamObject == gameObject;
        });
    GameObjectLocomition* locomotionState = nullptr;
    if (object_iter != mGameObjectEntries.end())
    {
        locomotionState = object_iter->mLocomotionState.get();
    }
    return locomotionState;
}

cxx::uniqueptr<GameObjectLocomition> LocomotionService::CreateGameObjectLocomotion()
{
    static SimplePool<GameObjectLocomition> objectsPool = (
        [](GameObjectLocomition* object)
        {
            object->OnRecycle();
        });

    GameObjectLocomition* objectPtr = objectsPool.Acquire();
    return std::move(cxx::uniqueptr<GameObjectLocomition> (objectPtr, [](GameObjectLocomition* object)
    {
        if (object)
        {
            objectsPool.Return(object);
        }
    }));
}

void LocomotionService::ProcessLocomotion(GameObjectLocomition* locomotion, PhysicsObject* physics)
{
    if (locomotion->IsOrienting())
    {
        cxx::angle_t currOrientation = physics->GetOrientation();

        float diff = cxx::wrap_angle_to_pi((locomotion->mDesiredOrientation - currOrientation).to_radians());
        float diff_abs = glm::abs(diff);

        // finish?
        if (diff_abs <= locomotion->mAlignTolerance)
        {
            locomotion->StopOrient();
            physics->ClearAngularVelocity();
        }
        else
        {
            // how much we SHOULD turn this frame
            float turn_step = mSimulationStepDelta * locomotion->mOrientSpeed;
            if (turn_step > diff_abs)
            {
                turn_step = diff_abs;
            }

            float angularVelocity = (turn_step / mSimulationStepDelta) * glm::sign(diff);
            physics->SetAngularVelocity(angularVelocity);
        }
    }

    // wait until rotation is done before move
    if (locomotion->IsOrienting()) 
    {
        physics->ClearLinearVelocity();
        return;
    }

    if (locomotion->IsMoving())
    {
        glm::vec2 currPosition = physics->GetPosition2d();
        glm::vec2 directionToTarget = (locomotion->mDesiredPosition - currPosition);

        float distanceToTarget = glm::length(directionToTarget);
        // finish?
        if (distanceToTarget <= locomotion->mArrivalTolerance)
        {
            locomotion->StopMovement();
            physics->ClearLinearVelocity();
        }
        else
        {
            glm::vec2 unitDirection = directionToTarget / distanceToTarget;

            float distanceToMove = mSimulationStepDelta * locomotion->mArriveSpeed;
            if (distanceToMove > distanceToTarget)
            {
                distanceToMove = distanceToTarget;
            }

            glm::vec2 linearVelocity = unitDirection * distanceToMove / mSimulationStepDelta;

            physics->SetLinearVelocity(linearVelocity);
        }
    }
}

/*
// obsolete

void LocomotionService::ProcessGameObjects(float deltaTime)
{
    for (const GameObjectEntry& roller: mGameObjectEntries)
    {
        GameObjectLocomition* locomotion = roller.mLocomotionState.get();
        GameObject* gameObject = roller.mGamObject;

        if (!locomotion->IsActive()) continue;

        if (locomotion->IsRotating())
        {
            float objectOrientation = gameObject->GetObjectOrientation().to_radians();
            float maxStep = locomotion->mRotateSpeed * deltaTime;
            float angleDiff = locomotion->mDesiredOrientation.to_radians() - objectOrientation;
            angleDiff = cxx::wrap_angle_to_pi(angleDiff);
            // rotation done ?
            if (cxx::eps_equals(glm::abs(angleDiff), 0.0f) || (glm::abs(angleDiff) <= maxStep))
            {
                objectOrientation = locomotion->mDesiredOrientation.to_radians();
                locomotion->StopRotation();
            }
            else // continue rotate
            {
                float direction = glm::sign(angleDiff);
                objectOrientation = cxx::wrap_angle_to_pi(objectOrientation + direction * maxStep);
            } 
            gameObject->SyncWithLocomotionOrientation(cxx::angle_t::from_radians(objectOrientation));
        }

        // wait until rotation is done before move
        if (locomotion->IsRotating()) continue;

        // todo: collision checking?
        // todo: floor position

        if (locomotion->IsMoving())
        {
            glm::vec2 objectPosition = gameObject->GetObjectPosition2d();
            glm::vec2 directionToTarget = (locomotion->mDesiredPosition - objectPosition);

            float distanceToTarget = glm::length(directionToTarget);
            // motion done?
            if (cxx::eps_equals(distanceToTarget, 0.0f))
            {
                objectPosition = locomotion->mDesiredPosition;
                locomotion->StopMovement();
            }
            else // continue move
            {
                directionToTarget /= distanceToTarget; // normalize
                float distanceToMove = glm::min(locomotion->mMoveSpeed * deltaTime, distanceToTarget);
                objectPosition += directionToTarget * distanceToMove;
            }
            gameObject->SyncWithLocomotionPosition(objectPosition);
        }
    }
}
*/
