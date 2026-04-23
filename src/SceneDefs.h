#pragma once

//////////////////////////////////////////////////////////////////////////

class Scene;
class SceneObject;
class EnvironmentMeshObject;
class AnimatingMeshObject;
class ProceduralMeshObject;

//////////////////////////////////////////////////////////////////////////

struct SceneRenderLists
{
public:
    //////////////////////////////////////////////////////////////////////////
    struct Entry
    {
    public:
        float mDistanceToCamera2 = 0.0f; // squared

        // switch
        EnvironmentMeshObject* mWaterLavaMesh = nullptr;
        AnimatingMeshObject* mAnimatingMesh = nullptr;
        ProceduralMeshObject* mProceduralMesh = nullptr;
    };
    //////////////////////////////////////////////////////////////////////////
public:
    SceneRenderLists() = default;
    // reset lists
    inline void Clear()
    {
        for (int icouner = 0; icouner < eRenderPass_COUNT; ++icouner)
        {
            mListsPerPass[icouner].clear();
        }
    }
    // queue
    inline void Register(eRenderPass renderPass, EnvironmentMeshObject* object, float distanceToCamera2)
    {
        if (object == nullptr) return;
        
        Entry& objectEntry = mListsPerPass[renderPass].emplace_back();
        objectEntry.mWaterLavaMesh = object;
        objectEntry.mDistanceToCamera2 = distanceToCamera2;
    }
    inline void Register(eRenderPass renderPass, AnimatingMeshObject* object, float distanceToCamera2)
    {
        if (object == nullptr) return;

        Entry& objectEntry = mListsPerPass[renderPass].emplace_back();
        objectEntry.mAnimatingMesh = object;
        objectEntry.mDistanceToCamera2 = distanceToCamera2;
    }
    inline void Register(eRenderPass renderPass, ProceduralMeshObject* object, float distanceToCamera2)
    {
        if (object == nullptr) return;

        Entry& objectEntry = mListsPerPass[renderPass].emplace_back();
        objectEntry.mProceduralMesh = object;
        objectEntry.mDistanceToCamera2 = distanceToCamera2;
    }
public:
    std::vector<Entry> mListsPerPass[eRenderPass_COUNT];
};