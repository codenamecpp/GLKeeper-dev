#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ScenarioDefs.h"

//////////////////////////////////////////////////////////////////////////

class DK2ScenarioReader: public cxx::noncopyable
{
private:

    //////////////////////////////////////////////////////////////////////////

    using LevelDataFileID = unsigned int;

    struct LevelDataFilePath
    {
    public:
        LevelDataFilePath() = default;
        LevelDataFilePath(LevelDataFileID aDataTypeId, const std::string& aFileName)
            : mId(aDataTypeId)
            , mFilePath(aFileName)
        {}
    public:
        LevelDataFileID mId;
        std::string mFilePath;
    };

    struct KWDFileHeader
    {
    public:
        unsigned int mFileSize;
        unsigned int mContentSize;
        unsigned int mHeaderEndOffset;
        int mItemsCount;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    DK2ScenarioReader() = default;

    bool ReadScenarioData(const std::string& filePath, ScenarioDefinition& scenarioData);
    bool ReadGlobalData(ScenarioDefinition& scenarioData);

private:
    void ApplyExtensions(ScenarioDefinition& scenarioData) const;
    bool ExploreTerrainTypes(ScenarioDefinition& scenarioData) const;
    bool ReadDataFile(LevelDataFileID dataTypeId, ScenarioDefinition& scenarioData);
    bool ReadMapInfo(ScenarioDefinition& scenarioData, std::vector<LevelDataFilePath>& paths);
    bool ReadLevelVariables(ScenarioDefinition& scenarioData);
    bool ReadTerrainData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadTerrainDefinition(TerrainDefinition& terrainDef);
    bool ReadCreaturesData(const KWDFileHeader& header, ScenarioDefinition& scenarioData);
    bool ReadCreaturesDefinition(const KWDFileHeader& header, CreatureDefinition& creature);
    bool ReadRoomsData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadRoomDefinition(RoomDefinition& roomDef);
    bool ReadPlayersData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadPlayerDefinition(PlayerDefinition& playerDef);
    bool ReadObjectThingData(ScenarioObjectThing& objectThingData);
    bool ReadThingsData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadObjectsData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadObjectDefinition(GameObjectDefinition& objectDef);
    bool ReadMapData(ScenarioDefinition& scenarioData);
    bool ReadScenarioVariables(int numElements, ScenarioDefinition& scenarioData);
    bool ReadStringId();
    bool ReadObjectFlags(GameObjectDefinition& objectDef);
    bool ReadRoomFlags(RoomDefinition& roomDef);
    bool ReadTerrainFlags(TerrainDefinition& terrainDef);
    bool ReadArtResource(ArtResourceDefinition& artResource);
    bool ReadLight();
    bool Read32bitsFloat(float& outputFloat);
    bool ReadTimestamp();
    bool ReadString8(unsigned int stringLength, std::string& ansiString);
    bool ReadString(unsigned int stringLength, std::wstring& wideString);

private:
    std::ifstream mFileStream;
};

//////////////////////////////////////////////////////////////////////////