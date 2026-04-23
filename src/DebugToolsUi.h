#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ToolsUi.h"
#include "RenderView.h"
#include "AnimatingMeshObject.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class DebugToolsUi : public ToolsUi, public GameSessionAware
{
private:
    // override ToolsUi
    void DoUI(ImGuiIO& imguiContext, float deltaTime) override;
    void OnActivateUi() override;
    void OnDeactivateUi() override;

private:
    void DoCheatsTab(ImGuiIO& imguiContext);
    
    void EnableMeshPreview();
    void DisableMeshPreview(bool forceUnloadResources = false);
    void SetMeshPreview(const ArtResourceDefinition& def);

    void DoObjectDefsTab(ImGuiIO& imguiContext);
    void DoArtResourceDef(ImGuiIO& imguiContext, const ArtResourceDefinition& def);

    void SetupObjectDefs();
    void ClearObjectDefs();

    void ConfigurePreviewCameraProjection(Camera& camera);

    void ChangeGoldAmount(long amount);

private:
    cxx::uniqueptr<AnimatingMeshObject> mPreviewMeshObject;
    RenderView* mPreviewMeshRenderView = nullptr;

    bool mIsMeshPreviewActive = false;

    cxx::angle_t mPreviewMeshRotation;

    // scenario objects
    std::vector<GameObjectDefinition> mObjectDefsList;
};