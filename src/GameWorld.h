#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameWorldDefs.h"
#include "SceneObject.h"
#include "TileSelectionOutline.h"
#include "GameMap.h"
#include "TileConstructor.h"
#include "Scene.h"
#include "TileConstructionSet.h"
#include "GameObjectManager.h"
#include "RoomManager.h"
#include "LocomotionService.h"
#include "Physics.h"

//////////////////////////////////////////////////////////////////////////

class GameWorld: public GameSessionAware
{
public:
    // Load session data, level map, setup players, build rooms etc
    bool LoadScenario(const ScenarioDefinition& scenarioDefinition, GameLoadingAware& loadingContext);
    // on start / end of each game session
    void EnterWorld();
    void ClearWorld();

    // update world
    void UpdateFrame(float deltaTime);
    void UpdateLogicTick(float stepDeltaTime);
    void UpdatePhysicsTick(float stepDeltaTime);

    // accessing scene
    inline Scene& GetScene() { return mScene; }

    // accessing primary camera state
    inline Camera& GetMainCamera() { return GetScene().GetCamera(); }

    // accessing map info and tile data
    inline GameMap& GetGameMap() { return mGameMap; }

    // accessing dungeon builder
    inline TileConstructionSet& GetTileConstructionSet() { return mTileConstructionSet; }

    // accessing tile selection outline
    inline TileSelectionOutline& GetTileSelectionOutline() { return mTileSelectionOutline; }

    // accessing current world statistics
    const WorldStatistics& GetStatistics() const { return mPrevFrameStats; }

    // accessing game objects
    inline GameObjectManager& GetGameObjects() { return mGameObjects; }

    // accessing room manager
    inline RoomManager& GetRoomManager() { return mRoomManager; }

    // accessing locomotion service
    inline LocomotionService& GetLocomotionService() { return mLocomotionService; }

    // accessing world physics
    inline Physics& GetPhysics() { return mPhysics; }

    // Test whether room is buildable on specific terrain spot
    bool CanPlaceRoomOnLocation(MapTile* mapTile, ePlayerID playerID, RoomDefinition* roomDefinition) const;

    // Test whether room is sellable on specific terrain spot
    bool CanSellRoomOnLocation(MapTile* mapTile, ePlayerID playerID) const;

    // Set terrain state tagged
    void TagTerrain(const MapArea2D& tileArea);

    // Reset terrain tagged state
    void UnTagTerrain(const MapArea2D& tileArea);

    // Sell rooms and objects within specified area, will split rooms
    void SellEntities(ePlayerID playerID, const MapArea2D& tilesArea);

    // Build room within specified area, will merge contiguous rooms
    void ConstructRoom(ePlayerID playerID, RoomDefinition* roomDefinition, const MapArea2D& tilesArea);

    // Do damage terrain tile, it will also damage room at this location
    void DamageTerrainTile(MapTile* mapTile, ePlayerID playerID, int hitPoints);

    // Repair or claim terrain tile, it will also fix walls of surrounding rooms
    void RepairTerrainTile(MapTile* mapTile, ePlayerID playerID, int hitPoints);

    // Cast ray in specific viewport coordinate using current camera
    bool CastRayFromScreenPoint(const Point2D& screenCoordinate, cxx::ray3d_t& resultRay);

    // When tile geometry gets modified it must then be synchronized with physics and rendering systems, its need to be manually invalidated
    // Geometry of invalidated tiles will be Clear and then Reconstructed
    // Rooms themselves must handle invalidated tiles, Reconstruct it geometry, but without Clear
    void InvalidateTerrainTile(MapTile* mapTile);
    void InvalidateTerrainTile(MapTile* mapTile, eTileFace face);
    // Full invalidation (all faces) of tiles in array
    void InvalidateTerrainTiles(cxx::span<MapTile*> mapTiles);
    // Invalidate tiles surrounding target tile in all directions
    // If neighbour tile is solid, single wall side will be invalidated but if solid tile is in diagonal direction it will be skipped
    void InvalidateTerrainTileNeighbours(MapTile* mapTile);

    // generate new entity unique id
    EntityUid GenerateEntityUid();

private:
    template<typename TContainer, typename TFilterFunc>
    void ScanAdjacentRooms(cxx::span<MapTile*> tilesToScan, ePlayerID ownerId, TFilterFunc filterFunc, TContainer& container) const;

    template<typename TEnumProc>
    void EnumRoomSegments(Room* roomInstance, TEnumProc enumProc);

    // Explore room area and create room instance from it
    void CreateRoomFromUnexploredTiles(MapTile* startTile);

    void CreateEnvironmentEntities();

    // Tiles will be no more part of specified room instance
    // Will create additional rooms for separated parts of original room
    void ReleaseRoomTiles(Room* roomInstance, cxx::span<MapTile*> roomTiles);

    void HandleRoomAbsorbed(Room* roomInstance);
    void HandleRoomCollapsed(Room* roomInstance);

    void BuildDirtyTerrain();
    void ResetInvalidatedTerrain();

    // force recompute heightmap of all map tiles
    void InitTilesFloorHeightmap();
    void UpdateTileFloorHeightmap(MapTile* mapTile);

    void SetTileTagged(MapTile* mapTile, bool isTagged);

private:
    Scene mScene;

    EntityUid mNextEntityUid = 1;

    TileSelectionOutline mTileSelectionOutline;
    TileConstructionSet mTileConstructionSet;

    std::vector<MapTile*> mInvalidatedTerrainTiles;

    GameMap mGameMap;
    GameObjectManager mGameObjects;
    RoomManager mRoomManager;
    LocomotionService mLocomotionService;
    Physics mPhysics;

    WorldStatistics mCurrentFrameStats;
    WorldStatistics mPrevFrameStats;

    std::vector<cxx::uniqueptr<EnvironmentMeshObject>> mEnvironmentObjects;
};

//////////////////////////////////////////////////////////////////////////