#include "stdafx.h"
#include "DebugToolsUi.h"
#include "imgui.h"
#include "GameWorld.h"
#include "GameRenderManager.h"
#include "MeshAssetManager.h"
#include "GameMain.h"

void DebugToolsUi::DoUI(ImGuiIO& imguiContext, float deltaTime)
{
    const ImVec2 initialSize { 400.0f, 200.0f };

    ImGui::SetNextWindowSize(initialSize, ImGuiCond_Once);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);

    if (!ImGui::Begin("Debug Tools UI"))
    {
        DisableMeshPreview();

        ImGui::End();
        return;
    }

    EnableMeshPreview();

    if (ImGui::BeginTabBar("Tabs"))
    {
        if (ImGui::BeginTabItem("Cheats"))
        {
            DoCheatsTab(imguiContext);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Defs"))
        {

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Objs"))
        {
            DoObjectDefsTab(imguiContext);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    mPreviewMeshRotation += cxx::angle_t::from_degrees(-30.0f * deltaTime);
    if (mPreviewMeshObject && mPreviewMeshObject->IsActive())
    {
        mPreviewMeshObject->ResetOrientation();
        mPreviewMeshObject->RotateAroundAxis(WorldAxes::Y, mPreviewMeshRotation);
    }
}

void DebugToolsUi::OnActivateUi()
{
    EnableMeshPreview();
    SetupObjectDefs();
    DisableMeshPreview();
}

void DebugToolsUi::OnDeactivateUi()
{
    DisableMeshPreview(true);
    ClearObjectDefs();
}

void DebugToolsUi::DoCheatsTab(ImGuiIO& imguiContext)
{
    ImGui::PushID("cheats");

    if (ImGui::CollapsingHeader("Scene"))
    {
        ImGui::Checkbox("Show objects Aabb", &gDebug.mDrawSceneAabbTree);
        ImGui::Checkbox("Hide objects", &gDebug.mNoDrawSceneObjects);
    }

    if (ImGui::CollapsingHeader("Economy"))
    {
        if (ImGui::Button("+100 Gold"))
        {
            ChangeGoldAmount(100);
        }
        if (ImGui::Button("+1.000 Gold"))
        {
            ChangeGoldAmount(1000);
        }

        if (ImGui::Button("-100 Gold"))
        {
            ChangeGoldAmount(-100);
        }
        if (ImGui::Button("-1.000 Gold"))
        {
            ChangeGoldAmount(-1000);
        }
    }

    if (ImGui::Button("Spawn Chicken"))
    {
        EntityHandle objectHandle = GetObjectManager().CreateObject(GameObjectClassId_Chicken);
        if (GameObject* gameObject = GetObjectManager().GetObjectPtr(objectHandle))
        {
            gameObject->SetObjectPosition({4.0f, 1.0f, 18.0f});
        }
        GetObjectManager().ActivateObject(objectHandle);
    }

    
    ImGui::PopID();
}

void DebugToolsUi::EnableMeshPreview()
{
    if (mIsMeshPreviewActive) return;

    mIsMeshPreviewActive = true;

    // create mesh preview object
    if (mPreviewMeshObject == nullptr)
    {
        mPreviewMeshObject = GetScene().CreateAnimatingMesh();
        cxx_assert(mPreviewMeshObject);
        mPreviewMeshObject->SetActive(true);
    }

    // create render view 
    if (mPreviewMeshRenderView == nullptr)
    {
        mPreviewMeshRenderView = gGameRenderer.CreateRenderView();
        cxx_assert(mPreviewMeshRenderView);
    }

    if ((mPreviewMeshObject == nullptr) || (mPreviewMeshRenderView == nullptr)) 
    {
        cxx_assert(false);
        return;
    }

    mPreviewMeshObject->SetPosition({0.0f, 0.0f, 0.0f});
    mPreviewMeshObject->SetRenderLayers(RenderLayer_MeshPreview);
    mPreviewMeshObject->SetActive(true);

    Camera& previewCamera = mPreviewMeshRenderView->GetCamera();
    ConfigurePreviewCameraProjection(previewCamera);
    previewCamera.SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
    previewCamera.LookAt(mPreviewMeshObject->GetPosition(), WorldAxes::Y);
    previewCamera.Translate(glm::vec3{0.0f, 0.3f, 0.0f});
    previewCamera.mRenderLayersMask = RenderLayer_MeshPreview;
    mPreviewMeshRenderView->SetActive(true);
}

void DebugToolsUi::DisableMeshPreview(bool forceUnloadResources)
{
    if (forceUnloadResources)
    {
        // destroy mesh preview object
        mPreviewMeshObject.reset();

        if (mPreviewMeshRenderView)
        {
            gGameRenderer.DestroyRenderView(mPreviewMeshRenderView);
            mPreviewMeshRenderView = nullptr;
        }
    }

    if (!mIsMeshPreviewActive) return;

    mIsMeshPreviewActive = false;

    if (mPreviewMeshRenderView)
    {
        mPreviewMeshRenderView->SetActive(false);
    }
}

void DebugToolsUi::SetMeshPreview(const ArtResourceDefinition& def)
{
    EnableMeshPreview();

    mPreviewMeshRotation = {};

    if ((def.mResourceType == eArtResource_Mesh) ||
        (def.mResourceType == eArtResource_TerrainMesh) ||
        (def.mResourceType == eArtResource_AnimatingMesh))
    {
        MeshAsset* meshAsset = gMeshAssetManager.GetMesh(def.mResourceName);
        cxx_assert(meshAsset);

        cxx_assert(mPreviewMeshObject);
        if (def.mResourceType == eArtResource_AnimatingMesh)
        {
            AnimatingMeshObject::AnimationParams animParams;
                animParams.mFirstFrame = 0;
                animParams.mLastFrame = def.mAnimationDesc.mFrames - 1;
                animParams.mIsLoopEnabled = def.mDoesntLoop == false;
                animParams.mFramesPerSecond = def.mAnimationDesc.mFps * 1.0f;

            mPreviewMeshObject->Configure(meshAsset, animParams);
        }
        else
        {
            mPreviewMeshObject->Configure(meshAsset);
        }
    }
    else
    {
        DisableMeshPreview();
    }
}

void DebugToolsUi::DoObjectDefsTab(ImGuiIO& imguiContext)
{
    ImGui::PushID("obj_defs");

    int objectsCounter = 0;
    for (const GameObjectDefinition& roller: mObjectDefsList)
    {
        int objectIndex = objectsCounter++;

        if (roller.mObjectName.empty()) continue; // skip dummy

        ImGui::PushID(objectIndex);

        if (ImGui::CollapsingHeader(cxx::va("%s [#%d]", roller.mObjectName.c_str(), objectIndex)))
        {
            if (ImGui::TreeNode("Physics props"))
            {
                ImGui::Value("Width", roller.mWidth); 
                ImGui::Value("Height", roller.mHeight);
                ImGui::Value("Mass", roller.mMass); 
                ImGui::Value("Speed", roller.mSpeed);
                ImGui::Value("Air Friction", roller.mAirFriction);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Object flags"))
            {
                if (roller.mDieOverTime) { ImGui::BulletText("Die over time"); }
                if (roller.mDieOverTimeIfNotInRoom) { ImGui::BulletText("Die over time if not in room"); }

                ImGui::BulletText("Category: %s", cxx::enum_to_string(roller.mObjectCategory));

                if (roller.mCanBePickedUp) { ImGui::BulletText("Can be picked up"); }
                if (roller.mCanBeSlapped) { ImGui::BulletText("Can be slapped"); }
                if (roller.mDieWhenSlapped) { ImGui::BulletText("Die when slapped"); }
                if (roller.mCanBeDroppedOnAnyLand) { ImGui::BulletText("Can be dropped on any land"); }
                if (roller.mObstacle) { ImGui::BulletText("Obstacle"); }
                if (roller.mBounce) { ImGui::BulletText("Bounce"); }
                if (roller.mBoulderCanRollThrough) { ImGui::BulletText("Boulder can roll through"); }
                if (roller.mBoulderDestroys) { ImGui::BulletText("Boulder destroys"); }
                if (roller.mIsPillar) { ImGui::BulletText("Pillar"); }
                if (roller.mDoorKey) { ImGui::BulletText("Door key"); }
                if (roller.mIsDamageable) { ImGui::BulletText("Damageable"); }
                if (roller.mHighlightable) { ImGui::BulletText("Highlightable"); }
                if (roller.mPlaceable) { ImGui::BulletText("Placeable"); }
                if (roller.mFirstPersonObstacle) { ImGui::BulletText("First person obstacle"); }
                if (roller.mSolidObstacle) { ImGui::BulletText("Solid obstacle"); }
                if (roller.mCastShadows) { ImGui::BulletText("Cast shadows"); }
                ImGui::TreePop();
            }

            if (roller.mResourceMesh.IsDefined() && ImGui::TreeNode("Resource: Mesh"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceMesh);
                ImGui::TreePop();
            }

            if (roller.mResourceGuiIcon.IsDefined() && ImGui::TreeNode("Resource: Gui Icon"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceGuiIcon);
                ImGui::TreePop();
            }

            if (roller.mResourceInHandIcon.IsDefined() && ImGui::TreeNode("Resource: In Hand Icon"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceInHandIcon);
                ImGui::TreePop();
            }

            if (roller.mResourceInHandMesh.IsDefined() && ImGui::TreeNode("Resource: In Hand Mesh"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceInHandMesh);
                ImGui::TreePop();
            }

            if (roller.mResourceUnknown.IsDefined() && ImGui::TreeNode("Resource: Unknown"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceUnknown);
                ImGui::TreePop();
            }

            if (roller.mResourceAdditional1.IsDefined() && ImGui::TreeNode("Resource: Additional 1"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceAdditional1);
                ImGui::TreePop();
            }

            if (roller.mResourceAdditional2.IsDefined() && ImGui::TreeNode("Resource: Additional 2"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceAdditional2);
                ImGui::TreePop();
            }

            if (roller.mResourceAdditional3.IsDefined() && ImGui::TreeNode("Resource: Additional 3"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceAdditional3);
                ImGui::TreePop();
            }

            if (roller.mResourceAdditional4.IsDefined() && ImGui::TreeNode("Resource: Additional 4"))
            {
                DoArtResourceDef(imguiContext, roller.mResourceAdditional4);
                ImGui::TreePop();
            }
        }

        ImGui::PopID();
    }

    ImGui::PopID();
}

void DebugToolsUi::DoArtResourceDef(ImGuiIO& imguiContext, const ArtResourceDefinition& def)
{
    if (!def.IsDefined()) return;

    ImGui::Text("Resource type: %s", cxx::enum_to_string(def.mResourceType));
    if ((def.mResourceType == eArtResource_Mesh) ||
        (def.mResourceType == eArtResource_TerrainMesh) ||
        (def.mResourceType == eArtResource_AnimatingMesh))
    {
        ImGui::SameLine();
        if (ImGui::Button("Preview"))
        {
            SetMeshPreview(def);
        }
    }
    ImGui::Text("Resource name: %s", def.mResourceName.c_str());
    ImGui::Text("Start / End AF: %d / %d", def.mStartAF, def.mEndAF);

    if ((def.mResourceType == eArtResource_Sprite) ||
        (def.mResourceType == eArtResource_Alpha) ||
        (def.mResourceType == eArtResource_AdditiveAlpha))
    {
        ImGui::Text("Image Frames: %d", def.mImageDesc.mFrames);
        ImGui::Text("Image Size: %f x %f", def.mImageDesc.mWidth, def.mImageDesc.mHeight);
    }

    if ((def.mResourceType == eArtResource_Mesh) ||
        (def.mResourceType == eArtResource_MeshCollection))
    {
        ImGui::Text("Mesh Frames: %d", def.mMeshDesc.mFrames);
        ImGui::Text("Mesh Scale: %f", def.mMeshDesc.mScale);
    }

    if (def.mResourceType == eArtResource_TerrainMesh)
    {
        ImGui::Text("Terrain Frames: %d", def.mTerrainDesc.mFrames);
    }

    if (def.mResourceType == eArtResource_ProceduralMesh)
    {
        ImGui::Text("Proc Id: %d", def.mProcDesc.mId);
    }

    if (def.mResourceType == eArtResource_AnimatingMesh)
    {
        ImGui::Text("Anim Frames: %d", def.mAnimationDesc.mFrames);
        ImGui::Text("Anim Fps: %d", def.mAnimationDesc.mFps);
        ImGui::Text("Anim Dist Start / End: %d / %d", def.mAnimationDesc.mDistStart, def.mAnimationDesc.mDistEnd);
    }

    if (ImGui::TreeNode("Flags"))
    {
        if (def.mPlayerColoured) { ImGui::BulletText("Player coloured"); }
        if (def.mAnimatingTexture) { ImGui::BulletText("Animating texture"); }
        if (def.mHasStartAnimation) { ImGui::BulletText("Has start animation"); }
        if (def.mHasEndAnimation) { ImGui::BulletText("Has end animation"); }
        if (def.mRandomStartFrame) { ImGui::BulletText("Random start frame"); }
        if (def.mOriginAtBottom) { ImGui::BulletText("Origin at bottom"); }
        if (def.mDoesntLoop) { ImGui::BulletText("Doesnt loop"); }
        if (def.mFlat) { ImGui::BulletText("Flat"); }
        if (def.mDoesntUseProgressiveMesh) { ImGui::BulletText("Doesnt use progressive mesh"); }
        if (def.mUseAnimatingTextureForSelection) { ImGui::BulletText("Use animating texture for selection"); }
        if (def.mPreload) { ImGui::BulletText("Preload"); }
        if (def.mBlood) { ImGui::BulletText("Blood"); }
        ImGui::TreePop();
    }
}

void DebugToolsUi::SetupObjectDefs()
{
    const ScenarioDefinition& scenarioData = GetScenarioDefinition();
    mObjectDefsList = scenarioData.mGameObjectDefs;
}

void DebugToolsUi::ClearObjectDefs()
{
    mObjectDefsList.clear();
}

void DebugToolsUi::ConfigurePreviewCameraProjection(Camera& camera)
{
    Camera::ProjectionParams cameraParams;
    cameraParams.mFarDistance = 10.0f;
    cameraParams.mNearDistance = 0.01f;
    cameraParams.mFovy = 30.0f;
    camera.SetupProjection(cameraParams);
}

void DebugToolsUi::ChangeGoldAmount(long amount)
{
    if (amount == 0) return;

    if (amount > 0)
    {
        GetEconomyService().GiveResource(GetGameSession().GetLocalPlayer(), eGameResource_Gold, amount);
    }
    else
    {
        GetEconomyService().TakeResource(GetGameSession().GetLocalPlayer(), eGameResource_Gold, -amount);
    }
}

