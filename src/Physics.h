#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObjectDefs.h"
#include "PhysicsObject.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class Physics final: public GameSessionAware
{
public:
    // start / finish game session
    bool LoadScenario(const ScenarioDefinition& scenarioDef);
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdatePhysicsTick(float stepDeltaTime);

    void AttachUser(EntityHandle entity, const EntityTransform& entityTransform, PhysicsTransformListener* listener);
    void DetachUser(EntityHandle entity);

    PhysicsObject* GetPhysicsObject(EntityHandle entity) const;

private:
    // factory
    cxx::uniqueptr<PhysicsObject> CreatePhysicsObject();

    void InterpolationStep(PhysicsObject* object, float t);
    void SimulationStep(PhysicsObject* object);
    void ResetVelocities(PhysicsObject* object);

private:
    float mSimulationStepDelta = 0.0f;
    float mInterpolationTime = 0.0f;

    //////////////////////////////////////////////////////////////////////////

    struct EntityEntry
    {
        EntityHandle mEntityHandle;
        cxx::uniqueptr<PhysicsObject> mPhysicsObject;
    };

    //////////////////////////////////////////////////////////////////////////

    std::vector<EntityEntry> mEntities;
    std::vector<PhysicsObject*> mObjects;
    std::vector<PhysicsObject*> mInterpolateTransforms;
};