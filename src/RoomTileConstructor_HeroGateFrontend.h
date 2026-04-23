#pragma once

#include "RoomTileConstructor.h"

class RoomTileConstructor_HeroGateFrontend: public RoomTileConstructor
{
public:
    RoomTileConstructor_HeroGateFrontend(TileConstructor& baseConstructor);
    // override RoomTileConstructor
    void ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles) override;
};