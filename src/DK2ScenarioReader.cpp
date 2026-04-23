#include "stdafx.h"
#include "DK2ScenarioReader.h"

//////////////////////////////////////////////////////////////////////////

#define DIVIDER_FLOAT 4096.0f
#define DIVIDER_DOUBLE 65536.0f

//////////////////////////////////////////////////////////////////////////

#define READ_FROM_FSTREAM(filestream, data) \
    if (!filestream.read(reinterpret_cast<char*>(&data), sizeof(data))) \
    { \
        return false; \
    }

#define READ_FSTREAM_DATATYPE(filestream, destination, dataType) \
    { \
        dataType _dataType; \
        if (!filestream.read(reinterpret_cast<char*>(&_dataType), sizeof(dataType))) \
        { \
            return false; \
        } \
        destination = static_cast<std::remove_reference<decltype(destination)>::type>(_dataType); \
    }

#define SKIP_FSTREAM_BYTES(filestream, numBytes) \
    { \
        if (!filestream.seekg(numBytes, std::ios::cur)) \
        { \
            return false; \
        } \
    }

#define READ_FSTREAM_BYTE(filestream, destination) \
    READ_FSTREAM_DATATYPE(filestream, destination, unsigned char)

#define READ_FSTREAM_WORD(filestream, destination) \
    READ_FSTREAM_DATATYPE(filestream, destination, unsigned short)

#define READ_FSTREAM_DWORD(filestream, destination) \
    READ_FSTREAM_DATATYPE(filestream, destination, unsigned int)

//////////////////////////////////////////////////////////////////////////

enum 
{
    DKLD_GLOBALS            = 0,
    DKLD_MAP                = 100,
    DKLD_TERRAIN            = 110,
    DKLD_ROOMS              = 120,
    DKLD_TRAPS              = 130,
    DKLD_DOORS              = 140,
    DKLD_KEEPER_SPELLS      = 150,
    DKLD_CREATURE_SPELLS    = 160,
    DKLD_CREATURES          = 170,
    DKLD_PLAYERS            = 180,
    DKLD_THINGS             = 190,
    DKLD_TRIGGERS           = 210,
    DKLD_LEVEL              = 220,
    DKLD_VARIABLES          = 230,
    DKLD_OBJECTS            = 240,
    DKLD_EFFECT_ELEMENTS    = 250,
    DKLD_SHOTS              = 260,
    DKLD_EFFECTS            = 270
};

//////////////////////////////////////////////////////////////////////////

enum
{
    DK_OBJECT_THING             = 194,
    DK_TRAP_THING               = 195,
    DK_DOOR_THING               = 196,
    DK_ACTIONPOINT_THING        = 197,
    DK_NEUTRAL_CREATURE_THING   = 198,
    DK_GOOD_CREATURE_THING      = 199,
    DK_CREATURE_THING           = 200,
    DK_HEROPARTY_THING          = 201,
    DK_DEAD_BODY_THING          = 202,
    DK_EFFECT_GENERATOR_THING   = 203,
    DK_ROOM_THING               = 204,
    DK_CAMERA_THING             = 205,
};

//////////////////////////////////////////////////////////////////////////

// ArtResource flags
enum 
{
    ARTRESF_PLAYER_COLOURED                     = (0x000002UL),
    ARTRESF_ANIMATING_TEXTURE                   = (0x000004UL),
    ARTRESF_HAS_START_ANIMATION                 = (0x000008UL),
    ARTRESF_HAS_END_ANIMATION                   = (0x000010UL),
    ARTRESF_RANDOM_START_FRAME                  = (0x000020UL),
    ARTRESF_ORIGIN_AT_BOTTOM                    = (0x000040UL),
    ARTRESF_DOESNT_LOOP                         = (0x000080UL),
    ARTRESF_FLAT                                = (0x000100UL),
    ARTRESF_DOESNT_USE_PROGRESSIVE_MESH         = (0x000200UL),
    ARTRESF_USE_ANIMATING_TEXTURE_FOR_SELECTION = (0x010000UL),
    ARTRESF_PRELOAD                             = (0x020000UL),
    ARTRESF_BLOOD                               = (0x040000UL)
};

//////////////////////////////////////////////////////////////////////////

enum 
{
    TERRAINF_SOLID                              = (0x00000001UL),
    TERRAINF_IMPENETRABLE                       = (0x00000002UL),
    TERRAINF_OWNABLE                            = (0x00000004UL),
    TERRAINF_TAGGABLE                           = (0x00000008UL),
    TERRAINF_ATTACKABLE                         = (0x00000020UL),
    TERRAINF_TORCH                              = (0x00000040UL), // has torch?
    TERRAINF_WATER                              = (0x00000080UL),
    TERRAINF_LAVA                               = (0x00000100UL),
    TERRAINF_ALWAYS_EXPLORED                    = (0x00000200UL), // doesnt used
    TERRAINF_PLAYER_COLOURED_PATH               = (0x00000400UL),
    TERRAINF_PLAYER_COLOURED_WALL               = (0x00000800UL),
    TERRAINF_CONSTRUCTION_TYPE_WATER            = (0x00001000UL),
    TERRAINF_CONSTRUCTION_TYPE_QUAD             = (0x00002000UL),
    TERRAINF_UNEXPLORE_IF_DUG_BY_ANOTHER_PLAYER = (0x00004000UL), // doesnt used
    TERRAINF_FILL_INABLE                        = (0x00008000UL),
    TERRAINF_ALLOW_ROOM_WALLS                   = (0x00010000UL),
    TERRAINF_DECAY                              = (0x00020000UL),
    TERRAINF_RANDOM_TEXTURE                     = (0x00040000UL),

    TERRAINF_TERRAIN_COLOR_R                    = (0x00080000UL), // expands R channel of lightColor
    TERRAINF_TERRAIN_COLOR_G                    = (0x00100000UL), // expands G channel of lightColor
    TERRAINF_TERRAIN_COLOR_B                    = (0x00200000UL), // expands B channel of lightColor

    TERRAINF_DWARF_CAN_DIG_THROUGH              = (0x00800000UL),
    TERRAINF_REVEAL_THROUGH_FOG_OF_WAR          = (0x01000000UL),

    TERRAINF_AMBIENT_COLOR_R                    = (0x02000000UL), // expands R channel of ambientLight
    TERRAINF_AMBIENT_COLOR_G                    = (0x04000000UL), // expands G channel of ambientLight
    TERRAINF_AMBIENT_COLOR_B                    = (0x08000000UL), // expands B channel of ambientLight

    TERRAINF_LIGHT                              = (0x10000000UL),
    TERRAINF_AMBIENT_LIGHT                      = (0x20000000UL),
};

//////////////////////////////////////////////////////////////////////////

enum 
{
    ROOMF_PLACEABLE_ON_WATER                    = (0x0001UL),
    ROOMF_PLACEABLE_ON_LAVA                     = (0x0002UL),
    ROOMF_PLACEABLE_ON_LAND                     = (0x0004UL),
    ROOMF_HAS_WALLS                             = (0x0008UL),
    ROOMF_CENTRE                                = (0x0010UL), // Placement
    ROOMF_SPECIAL_TILES                         = (0x0020UL), // Placement
    ROOMF_NORMAL_TILES                          = (0x0040UL), // Placement
    ROOMF_BUILDABLE                             = (0x0080UL),
    ROOMF_SPECIAL_WALLS                         = (0x0100UL), // Placement
    ROOMF_ATTACKABLE                            = (0x0200UL),
    ROOMF_UNK_0x0400                            = (0x0400UL),
    ROOMF_UNK_0x0800                            = (0x0800UL),
    ROOMF_HAS_FLAME                             = (0x1000UL),
    ROOMF_IS_GOOD                               = (0x2000UL)
};

//////////////////////////////////////////////////////////////////////////

enum 
{
    OBJECTF_DIE_OVER_TIME                       = (0x0000001UL),
    OBJECTF_DIE_OVER_TIME_IF_NOT_IN_ROOM        = (0x0000002UL),
    OBJECTF_TYPE_SPECIAL                        = (0x0000004UL),
    OBJECTF_TYPE_SPELL_BOOK                     = (0x0000008UL),
    OBJECTF_TYPE_CRATE                          = (0x0000010UL),
    OBJECTF_TYPE_LAIR                           = (0x0000020UL),
    OBJECTF_TYPE_GOLD                           = (0x0000040UL),
    OBJECTF_TYPE_FOOD                           = (0x0000080UL),
    OBJECTF_CAN_BE_PICKED_UP                    = (0x0000100UL),
    OBJECTF_CAN_BE_SLAPPED                      = (0x0000200UL),
    OBJECTF_DIE_WHEN_SLAPPED                    = (0x0000400UL),
    OBJECTF_TYPE_LEVEL_GEM                      = (0x0001000UL),
    OBJECTF_CAN_BE_DROPPED_ON_ANY_LAND          = (0x0002000UL),
    OBJECTF_OBSTACLE                            = (0x0004000UL),
    OBJECTF_BOUNCE                              = (0x0008000UL),
    OBJECTF_BOULDER_CAN_ROLL_THROUGH            = (0x0010000UL),
    OBJECTF_BOULDER_DESTROYS                    = (0x0020000UL),
    OBJECTF_PILLAR                              = (0x0040000UL),
    OBJECTF_DOOR_KEY                            = (0x0100000UL),
    OBJECTF_DAMAGEABLE                          = (0x0200000UL),
    OBJECTF_HIGHLIGHTABLE                       = (0x0400000UL),
    OBJECTF_PLACEABLE                           = (0x0800000UL),
    OBJECTF_FIRST_PERSON_OBSTACLE               = (0x1000000UL),
    OBJECTF_SOLID_OBSTACLE                      = (0x2000000UL),
    OBJECTF_CAST_SHADOWS                        = (0x4000000UL)
};

//////////////////////////////////////////////////////////////////////////

using ScenarioVariableType = unsigned int;
// well known variable types
enum : ScenarioVariableType
{
    ScenarioVariableType_Null = 0,
    ScenarioVariableType_CreaturePool = 1,
    ScenarioVariableType_Availability = 2,
    ScenarioVariableType_EntranceGenerationSpeedSeconds = 3,
    ScenarioVariableType_ClaimTileHealth = 4, // value=825
    ScenarioVariableType_AttackTileHealth = 5,
    ScenarioVariableType_RepairTileHealth = 6,
    ScenarioVariableType_MineGoldHealth = 7,
    ScenarioVariableType_DigRockHealth = 8,
    ScenarioVariableType_DigOwnWallHealth = 9,
    ScenarioVariableType_DigEnemyWallHealth = 10,
    ScenarioVariableType_FillInHealth = 11,
    ScenarioVariableType_ReinforceWallHealth = 12,
    ScenarioVariableType_RepairWallHealth = 13,
    ScenarioVariableType_ConvertRoomHealth = 14,
    ScenarioVariableType_AttackRoomHealth = 15,
    ScenarioVariableType_RepairRoomHealth = 16,
    // 17
    ScenarioVariableType_ChickenGenerationTimePerHatchery = 18,
    // 19
    ScenarioVariableType_GoldMinedFromGems = 20,
    ScenarioVariableType_TimeBetweenReattemptingHungerSeconds = 21,
    ScenarioVariableType_TimeBetweenReattemptingSleepSeconds = 22,
    ScenarioVariableType_DeadBodyDiesAfterSeconds = 23,
    ScenarioVariableType_MaxGoldPerTreasuryTile = 24,
    ScenarioVariableType_DecomposeValueNeededForVampire = 25,
    ScenarioVariableType_CreatureDyingStateDurationSeconds = 26,
    ScenarioVariableType_SpecialIncreaseGoldAmount = 27,
    ScenarioVariableType_SpecialIncreaseManaAmount = 28,
    // 29
    ScenarioVariableType_ImpIdleDelayBeforeReevaluationSeconds = 30,
    ScenarioVariableType_DelayBeforePayReevaluationSeconds = 31,
    ScenarioVariableType_MaxGoldPileOutsideTreasury = 32,
    ScenarioVariableType_PayDayFrequencySeconds = 33,
    ScenarioVariableType_MotionlessWhileResearchingSeconds = 34,
    ScenarioVariableType_MotionlessWhileManufacturingSeconds = 35,
    ScenarioVariableType_StateCounterDelayBeforeReattemptEating = 36,
    ScenarioVariableType_StateCounterWhileGuarding = 37,
    ScenarioVariableType_MotionlessWhilePrayingSeconds = 38,
    ScenarioVariableType_MotionlessInPrisonSeconds = 39,
    ScenarioVariableType_MotionlessWhileTrainingSeconds = 40,
    ScenarioVariableType_MotionlessInCasinoSeconds = 41,
    ScenarioVariableType_ExcessManaDecreaseRatePerSecond = 42,
    ScenarioVariableType_SacrificesId = 43,
    // 44
    ScenarioVariableType_DefaultHorizontalTerrainWibble = 45,
    ScenarioVariableType_DefaultVerticalTerrainWibble = 46,
    ScenarioVariableType_RebelMaxFollowers = 47,
    ScenarioVariableType_RebelLeavingPercentage = 48,
    ScenarioVariableType_RebelBecomeGoodPercentage = 49,
    ScenarioVariableType_RebelDefectPercentage = 50,
    ScenarioVariableType_TimeBeforeFirstPayDaySeconds = 51,
    ScenarioVariableType_DefaultTorchLightRed = 52,
    ScenarioVariableType_DefaultTorchLightGreen = 53,
    ScenarioVariableType_DefaultTorchLightBlue = 54,
    ScenarioVariableType_PrisonModifyCreatureHealthPerSecond = 55,
    // 56
    // 57
    // 58
    // 59
    // 60
    // 61
    // 62
    // 63
    // 64
    ScenarioVariableType_CreatureStatsId = 65,
    // 66
    ScenarioVariableType_DefaultCellingHeightTiles = 67,
    ScenarioVariableType_PayDayCutOffTimeSeconds = 68,
    // 69
    ScenarioVariableType_CasinoModifyCreatureGoldPerSecond = 70,
    ScenarioVariableType_CasinoBigWinKeeperLossPercentage = 71,
    // 72
    ScenarioVariableType_CreaturesSupportedByFirstPortal = 73,
    ScenarioVariableType_GameTicks = 74,
    ScenarioVariableType_ModifyHealthOfCreatureInLairPerSecond = 75,
    ScenarioVariableType_ModifyAngerOfCreatureInLairPerSecond = 76,
    // 77
    ScenarioVariableType_ModifyAngerInCompanyOfHatedCreaturesPerSecond = 78,
    ScenarioVariableType_ForceAppliedToSlappedCreature = 79,
    // 80
    // 81
    ScenarioVariableType_CannotSleepModifyCreatureHealthPerSecond = 82,
    ScenarioVariableType_CannotLeaveModifyCreatureHealthPerSecond = 83,
    ScenarioVariableType_PlayerRescanIntervalSeconds = 84,
    ScenarioVariableType_PlayerRescanRange = 85,
    ScenarioVariableType_MaxFreeRangeChickensPerPlayer = 86,
    ScenarioVariableType_WoodBridgeLifeOnLavaSeconds = 87,
    ScenarioVariableType_CreatureSleepsWhenBelowPercentHealth = 88,
    // 89
    // 90
    // 91
    // 92
    ScenarioVariableType_DoorPickedTimeSeconds = 93,
    ScenarioVariableType_MinimumImpThreshold = 94,
    ScenarioVariableType_TimeBeforeFreeImpGeneratedSeconds = 95,
    ScenarioVariableType_CreatureStunnedTimeSeconds = 96,
    ScenarioVariableType_CreatureStunnedEffectDelaySeconds = 97,
    ScenarioVariableType_FirstPersonStatIncreasePercentage = 98,
    // 99
    ScenarioVariableType_ManaCostInFirstPersonPerSecond = 100,
    ScenarioVariableType_HealthDecreaseWhenOnFirePerSecond = 101,
    ScenarioVariableType_ModifyCreatureAngerWhilePrayingPerSecond = 102,
    // 103
    ScenarioVariableType_RoomSellValuePercentageOfCost = 104,
    ScenarioVariableType_DoorSellValuePercentageOfCost = 105,
    ScenarioVariableType_TrapSellValuePercentageOfCost = 106,
    // 107
    // 108
    // 109
    ScenarioVariableType_InsufficientManaImpEvaluationPeriodSeconds = 110,
    ScenarioVariableType_DeathWhenPossessingManaReduction = 111,
    ScenarioVariableType_PercentageOfManaGainedBySacrificing = 112,
    ScenarioVariableType_PossessionFriendlyStatIncreasePercentage = 113,
    ScenarioVariableType_PossessionFriendlyHealthIncreasePercentage = 114,
    ScenarioVariableType_ModifyCreatureHealthWhilePrayingPerSecond = 115,
    ScenarioVariableType_HypnotismManaDrainPerSecond = 116,
    // 117
    // 118
    // 119
    // 120
    // 121
    // 122
    // 123
    // 124
    ScenarioVariableType_LowManaWarningThreshold = 125,
    ScenarioVariableType_TrainingRoomMaxExperienceLevel = 126,
    ScenarioVariableType_StunnedDamageIncreasePercentage = 127,
    ScenarioVariableType_MaxNumberOfThingsInHand = 128,
    ScenarioVariableType_RubberBandAreaLimit = 129,
    ScenarioVariableType_GravityConstant = 130,
    ScenarioVariableType_DungeonHeartHealthRegenerationPerSecond = 131,
    ScenarioVariableType_EPLossSecondWhileNotTraining = 132,
    ScenarioVariableType_ModifyAngerWhileSolitaryInPitPerSecond = 133,
    ScenarioVariableType_ModifyAngerOfCombatPitVictor = 134,
    ScenarioVariableType_ModifyAngerOfPitSpectatorPerSecond = 135,
    ScenarioVariableType_ConvertNeutralRoomHealth = 136,
    // 137
    ScenarioVariableType_ManufacturePointLossWhileNoWorkersPerSecond = 138,
    ScenarioVariableType_CasinoBigWinnerIncreasedHappinessPersentage = 139,
    ScenarioVariableType_DecreaseEntrancePersentageOfLastSackedCreatureType = 140,
    ScenarioVariableType_IncreaseEntrancePersentageOfNewCreatureTypes = 141,
    ScenarioVariableType_IncreasedWorkRatePersentageOfPlayersTorturedCreatureTypes = 142,
    ScenarioVariableType_IncreasedWorkRatePersentageWhenCasinoBigWinOccurs = 143,
    ScenarioVariableType_ImpPopCountdownTimeSeconds = 144,
    ScenarioVariableType_NumberOfRandomWallDeteriorationsPerSecond = 145,
    ScenarioVariableType_WallDeteriorationDamage = 146,
    ScenarioVariableType_WallDamageFromShot = 147,
    ScenarioVariableType_PercentageChanceOfKillingCreatureInPit = 148,
    ScenarioVariableType_ModifyPlayerColdWhileTrainingPerSecond = 149,
    ScenarioVariableType_SkeletonArmyDurationSeconds = 150,
    ScenarioVariableType_OvercrowdingJailBreakPercentagePerExtraCreature = 151,
    ScenarioVariableType_MaxTimeInGuardRoomBeforeWanderingToGuardPostSeconds = 152,
    // 153
    ScenarioVariableType_AmountOfGoldStolenInFirstPerson = 154,
    ScenarioVariableType_BoulderDeteriorationDamagePercentagePerSecond = 155,
    ScenarioVariableType_DrunkenDurationSeconds = 156,
    ScenarioVariableType_BoulderInitialHealth = 157,
    ScenarioVariableType_BoulderSpeedTilesPerSecond = 158,
    ScenarioVariableType_BoulderToCreatureDamagePercentage = 159,
    ScenarioVariableType_BoulderSpeedToHealthRatio = 160,
    ScenarioVariableType_BoulderFearAmount = 161,
    ScenarioVariableType_BoulderFearRange = 162,
    ScenarioVariableType_DrunkChancePercentagePerDrink = 163,
    ScenarioVariableType_IncreasedWorkRatePecentageFromSlapping = 164,
    ScenarioVariableType_IncreasedWorkRateDurationFromSlappingSeconds = 165,
    ScenarioVariableType_DungeonHeartMaxManaIncrease = 166,
    ScenarioVariableType_DungeonHeartManaGenerationIncreasePerSecond = 167,
    ScenarioVariableType_TremorDamagePerTerrainTilePerSecond = 168,
    ScenarioVariableType_ExternalGuardingDurationSeconds = 169,
    ScenarioVariableType_MinimumJobDurationSeconds = 170,
    // 171
    // 172
    ScenarioVariableType_CreatureCriticalHealthPercentageOfMax = 173,
    ScenarioVariableType_DungeonHeartObjectHealth = 174,
    ScenarioVariableType_ClaimNeutralManaVaultHealth = 175,
    ScenarioVariableType_ClaimEnemyManaVaultHealth = 176,
    ScenarioVariableType_CreaturesSupportedPerAdditionalPortal = 177,
    ScenarioVariableType_MaxGoldPerDungeonHeartTile = 178,
    ScenarioVariableType_ImpExperienceGainPerSecond = 179,
    ScenarioVariableType_DungeonHeartClaimScanRadiusTiles = 180,
    ScenarioVariableType_StunTimePercentageIncreaseWhenHit = 181,
    ScenarioVariableType_CallToArmsFightDistanceTiles = 182,
    ScenarioVariableType_DungeonHeartReportingDistanceTiles = 183,
    ScenarioVariableType_DwarfDiggingMultiplier = 184,
    ScenarioVariableType_TriggerTrapTriggerSpeedTilesPerSecond = 185,
    // 186
    ScenarioVariableType_FirstPersonAccuracyAngleDegrees = 187,
    ScenarioVariableType_SpecialReceiveImpsAmount = 188,
    ScenarioVariableType_DungeonHeartCreatureGatheringDistanceTiles = 189,
    ScenarioVariableType_GuardRoomReportingDistanceTiles = 190,
    ScenarioVariableType_GuardPostReportingDistanceTiles = 191,
    ScenarioVariableType_CombatPitMaxExperienceLevel = 192,
    ScenarioVariableType_TimeInHandBeforeCreaturesBecomeAngrySeconds = 193,
    ScenarioVariableType_SlapWorkFasterDurationSeconds = 194,
    ScenarioVariableType_SlapSpeedUpDurationSeconds = 195,
    ScenarioVariableType_TortureSpeedUpDurationSeconds = 196,
    ScenarioVariableType_CasinoWinSpeedUpDurationSeconds = 197,
    ScenarioVariableType_CriticalHealthPercentageForHeroLair = 198,
    ScenarioVariableType_GamblingModifyCreatureMoneySmilesPerSecond = 199,
    ScenarioVariableType_GamblingModifyCreatureMoneyMoneyPerSecond = 200,
    ScenarioVariableType_GamblingModifyCreatureAngerSmilesPerSecond = 201,
    ScenarioVariableType_GamblingModifyCreatureAngerMoneyPerSecond = 202,
    ScenarioVariableType_GamblingDelayBeforeBecomingFearlessSeconds = 203,
    ScenarioVariableType_GamblingFearlessDurationSeconds = 204,
    ScenarioVariableType_GamblingCreatureJackpotChanceSmilesPerSecond = 205,
    ScenarioVariableType_GamblingCreatureJackpotChanceMoneyPerSecond = 206,
    ScenarioVariableType_GamblingJackpotPayTimeLimit = 207,
    ScenarioVariableType_GamblingJackpotPayTimeLimitMissedAngerPercentageModifier = 208,
    ScenarioVariableType_GamblingJackpotBasicAmount = 209,
    ScenarioVariableType_FirstPersonRechargeModifyPercentage = 210,
    ScenarioVariableType_BackOffMinimumDurationSeconds = 211,
    ScenarioVariableType_BackOffMaximumDurationSeconds = 212,
    ScenarioVariableType_GoodCreaturesChaseEnemyDurationSeconds = 213,
    ScenarioVariableType_EvilCreaturesChaseEnemyDurationSeconds = 214,
    ScenarioVariableType_MusicLevelTwoThreatThreshold = 215,
    ScenarioVariableType_MusicLevelThreeThreatThreshold = 216,
    ScenarioVariableType_MusicLevelFourThreatThreshold = 217,
    ScenarioVariableType_TimeBeforeDungeonHeartConstructionBegins = 218,
    ScenarioVariableType_DefaultTorchLightIntensity = 219,
    ScenarioVariableType_DefaultTorchLightRadiusTiles = 220,
    ScenarioVariableType_DefaultTorchLightHeightTiles = 221,
    ScenarioVariableType_MaximumManaThreshold = 222,
    ScenarioVariableType_MaximumGroupNumberOfFollowers = 223,
    ScenarioVariableType_CreatureFirstPersonId = 224,
    ScenarioVariableType_CombatPitMeleeDamageModifierPercentage = 225,
    ScenarioVariableType_UpgradedPossessionCostPercentage = 226,
    ScenarioVariableType_UpgradedCallToArmsCostPercentage = 227,
    ScenarioVariableType_MPDScoreHeroKilled = 228,
    ScenarioVariableType_MPDScoreLandOwned = 229,
    ScenarioVariableType_MPDScoreGoldSlabsMined = 230,
    ScenarioVariableType_MPDScoreItemManufactured = 231,
    ScenarioVariableType_MPDScoreCreatureEntered = 232,
    // 233
    ScenarioVariableType_TortureChanceOfDyingWhenConverted = 234,
    ScenarioVariableType_MaximumManaGainPerSecond = 235,
    ScenarioVariableType_ClaimScanLightRed = 236,
    ScenarioVariableType_ClaimScanLightGreen = 237,
    ScenarioVariableType_ClaimScanLightBlue = 238,
    ScenarioVariableType_PitPercentageDamageTakenOfNormalCombat = 239,
    ScenarioVariableType_BoulderSlapDamage = 240,
    ScenarioVariableType_LevelRating = 241,
    ScenarioVariableType_AverageTime = 242,
};

//////////////////////////////////////////////////////////////////////////

inline bool KWDIntToENUM(unsigned int inputInt, ePlayerID& outputID)
{
    static const ePlayerID IDs[] =
    {
        ePlayerID_Null, // 0
        ePlayerID_Good, // 1
        ePlayerID_Neutral, // 2
        ePlayerID_Keeper1, // 3
        ePlayerID_Keeper2, // 4
        ePlayerID_Keeper3, // 5
        ePlayerID_Keeper4, // 6
    };
    if (inputInt < ePlayerID_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDIntToENUM(unsigned int inputInt, eBridgeTerrain& outputID)
{
    static const eBridgeTerrain IDs[] =
    {
        eBridgeTerrain_Null, // 0
        eBridgeTerrain_Water, // 1
        eBridgeTerrain_Lava, // 2
    };
    if (inputInt < eBridgeTerrain_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDIntToENUM(unsigned int inputInt, eGameObjectMaterial& outputID)
{
    static const eGameObjectMaterial IDs[] =
    {
        eGameObjectMaterial_None, // 0
        eGameObjectMaterial_Flesh, // 1
        eGameObjectMaterial_Rock, // 2
        eGameObjectMaterial_Wood, // 3
        eGameObjectMaterial_Metal1, // 4
        eGameObjectMaterial_Metal2, // 5
        eGameObjectMaterial_Magic, // 6
        eGameObjectMaterial_Glass, // 7
    };
    if (inputInt < eGameObjectMaterial_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDIntToENUM(unsigned int inputInt, eArtResource& outputID)
{
    static const eArtResource IDs[] =
    {
        eArtResource_Null, // 0
        eArtResource_Sprite, // 1
        eArtResource_Alpha, // 2
        eArtResource_AdditiveAlpha, // 3
        eArtResource_TerrainMesh, // 4
        eArtResource_Mesh, // 5
        eArtResource_AnimatingMesh, // 6
        eArtResource_ProceduralMesh, // 7
        eArtResource_MeshCollection, // 8
    };
    if (inputInt < eArtResource_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDIntToENUM(unsigned int inputInt, ePlayerType& outputID)
{
    static const ePlayerType IDs[] =
    {
        ePlayerType_Human, // 0
        ePlayerType_AI, // 1
    };
    if (inputInt < ePlayerType_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDIntToENUM(unsigned int inputInt, eComputerAI& outputID)
{
    static const eComputerAI IDs[] =
    {
        eComputerAI_MasterKeeper, // 0
        eComputerAI_Conqueror, // 1
        eComputerAI_Psychotic, // 2
        eComputerAI_Stalwart, // 3
        eComputerAI_Greyman, // 4
        eComputerAI_Idiot, // 5
        eComputerAI_Guardian, // 6
        eComputerAI_ThickSkinned, // 7
        eComputerAI_Paranoid, // 8
    };
    if (inputInt < eComputerAI_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDIntToENUM(unsigned int inputInt, eRoomTileConstruction& outputID)
{
    static const eRoomTileConstruction IDs[] =
    {
        eRoomTileConstruction_Complete,
        eRoomTileConstruction_Quad,
        eRoomTileConstruction_3_by_3,
        eRoomTileConstruction_3_by_3_Rotated,
        eRoomTileConstruction_Normal,
        eRoomTileConstruction_CenterPool,
        eRoomTileConstruction_DoubleQuad,
        eRoomTileConstruction_5_by_5_Rotated,
        eRoomTileConstruction_HeroGate,
        eRoomTileConstruction_HeroGateTile,
        eRoomTileConstruction_HeroGate_2_by_2,
        eRoomTileConstruction_HeroGateFrontend,
        eRoomTileConstruction_HeroGate_3_by_1,
    };

    if (inputInt == 0)
        return false;

    --inputInt; // shift down index
    if (inputInt < eRoomTileConstruction_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

bool DK2ScenarioReader::ReadString(unsigned int stringLength, std::wstring& wideString)
{
    wideString.resize(stringLength);

    // read bytes
    if (!cxx::read_elements(mFileStream, wideString.data(), stringLength))
        return false;

    // trim
    wideString.erase(
        std::find_if(wideString.begin(), wideString.end(), [](wchar_t c) -> bool {
            return c == 0;
        }), 
        wideString.end());

    return true;
}

bool DK2ScenarioReader::ReadString8(unsigned int stringLength, std::string& ansiString)
{
    ansiString.resize(stringLength);

    // read bytes
    if (!mFileStream.read(&ansiString[0], stringLength))
        return false;

    // trim
    ansiString.erase(
        std::find_if(ansiString.begin(), ansiString.end(), [](const char& c)->bool {
            return c == 0;
        }), 
        ansiString.end());

    return true;
}

bool DK2ScenarioReader::ReadTimestamp()
{
    unsigned short dkyear;
    unsigned char dkday;
    unsigned char dkmonth;
    unsigned char dkhour;
    unsigned char dkminute;
    unsigned char dksecond;
    unsigned char dkbyte;
    unsigned short fillerBytes;

    READ_FROM_FSTREAM(mFileStream, dkyear);
    READ_FROM_FSTREAM(mFileStream, dkday);
    READ_FROM_FSTREAM(mFileStream, dkmonth);
    READ_FROM_FSTREAM(mFileStream, fillerBytes);
    READ_FROM_FSTREAM(mFileStream, dkhour);
    READ_FROM_FSTREAM(mFileStream, dkminute);
    READ_FROM_FSTREAM(mFileStream, dksecond);
    READ_FROM_FSTREAM(mFileStream, dkbyte);

    return true;
}

bool DK2ScenarioReader::Read32bitsFloat(float& outputFloat)
{
    unsigned int encoded_float;
    READ_FSTREAM_DWORD(mFileStream, encoded_float);

    // decode float
    outputFloat = encoded_float / DIVIDER_FLOAT;
    return true;
}

bool DK2ScenarioReader::ReadLight()
{
    unsigned int ikpos;
    READ_FSTREAM_DWORD(mFileStream, ikpos); // x, / ConversionUtils.FLOAT
    READ_FSTREAM_DWORD(mFileStream, ikpos); // y, / ConversionUtils.FLOAT
    READ_FSTREAM_DWORD(mFileStream, ikpos); // z, / ConversionUtils.FLOAT

    unsigned int iradius;
    READ_FSTREAM_DWORD(mFileStream, iradius); // / ConversionUtils.FLOAT

    unsigned int flags;
    READ_FSTREAM_DWORD(mFileStream, flags);

    unsigned char rgb;
    READ_FSTREAM_BYTE(mFileStream, rgb); // r
    READ_FSTREAM_BYTE(mFileStream, rgb); // g
    READ_FSTREAM_BYTE(mFileStream, rgb); // b
    READ_FSTREAM_BYTE(mFileStream, rgb); // a

    return true;
}

bool DK2ScenarioReader::ReadArtResource(ArtResourceDefinition& artResource)
{
    if (!ReadString8(64, artResource.mResourceName))
        return false;

    unsigned int resourceFlags = 0;
    READ_FSTREAM_DWORD(mFileStream, resourceFlags);

    // reading flags
    artResource.mPlayerColoured = (resourceFlags & ARTRESF_PLAYER_COLOURED) > 0;
    artResource.mAnimatingTexture = (resourceFlags & ARTRESF_ANIMATING_TEXTURE) > 0;
    artResource.mHasStartAnimation = (resourceFlags & ARTRESF_HAS_START_ANIMATION) > 0;
    artResource.mHasEndAnimation = (resourceFlags &  ARTRESF_HAS_END_ANIMATION) > 0;
    artResource.mRandomStartFrame = (resourceFlags & ARTRESF_RANDOM_START_FRAME) > 0;
    artResource.mOriginAtBottom = (resourceFlags & ARTRESF_ORIGIN_AT_BOTTOM) > 0;
    artResource.mDoesntLoop = (resourceFlags & ARTRESF_DOESNT_LOOP) > 0;
    artResource.mFlat = (resourceFlags & ARTRESF_FLAT) > 0;
    artResource.mDoesntUseProgressiveMesh = (resourceFlags & ARTRESF_DOESNT_USE_PROGRESSIVE_MESH) > 0;
    artResource.mUseAnimatingTextureForSelection = (resourceFlags & ARTRESF_USE_ANIMATING_TEXTURE_FOR_SELECTION) > 0;
    artResource.mPreload = (resourceFlags & ARTRESF_PRELOAD) > 0;
    artResource.mBlood = (resourceFlags & ARTRESF_BLOOD) > 0;

    union packed_data_t
    {
        struct
        {
            unsigned int mEncodedWidth;
            unsigned int mEncodedHeight;
            unsigned short mFrames;
            unsigned short mNone_1;
        } mImageType;
        struct
        {
            unsigned int mEncodedScale;
            unsigned short mFrames;
            unsigned short mNone_1;
            unsigned int mNone_2;
        } mMeshType;
        struct
        {
            unsigned int mFrames;
            unsigned int mFps;
            unsigned short mStartDist;
            unsigned short mEndDist;
        } mAnimType;
        struct
        {
            unsigned int mId;
            unsigned int mNone_1;
            unsigned int mNone_2;
        } mProcType;
        struct
        {
            unsigned int mNone_1;
            unsigned int mNone_2;
            unsigned int mNone_3;
        } mTerrainType;
        unsigned char mBytes[12];
    };
    packed_data_t packed_data;
    static_assert(sizeof(packed_data) == 12, "Wrong size");
    if (!mFileStream.read(reinterpret_cast<char*>(&packed_data), sizeof(packed_data)))
        return false;

    // map resource type
    unsigned char resourceType;
    READ_FSTREAM_BYTE(mFileStream, resourceType);
    if (!KWDIntToENUM(resourceType, artResource.mResourceType))
        return false;

    if (artResource.mResourceType == eArtResource_AnimatingMesh)
    {
        READ_FSTREAM_BYTE(mFileStream, artResource.mStartAF);
        READ_FSTREAM_BYTE(mFileStream, artResource.mEndAF);
    }
    else
    {
        unsigned short unknownWord;
        READ_FSTREAM_WORD(mFileStream, unknownWord);
    }

    unsigned char unknownByte;
    READ_FROM_FSTREAM(mFileStream, unknownByte);

    switch (artResource.mResourceType)
    {
        case eArtResource_Sprite:
        case eArtResource_Alpha:
        case eArtResource_AdditiveAlpha:
            artResource.mImageDesc.mFrames = packed_data.mImageType.mFrames;
            artResource.mImageDesc.mWidth = packed_data.mImageType.mEncodedWidth / DIVIDER_FLOAT;
            artResource.mImageDesc.mHeight = packed_data.mImageType.mEncodedHeight / DIVIDER_FLOAT;
        break;

        case eArtResource_Mesh:
        case eArtResource_MeshCollection:
            artResource.mMeshDesc.mFrames = packed_data.mMeshType.mFrames;
            artResource.mMeshDesc.mScale = packed_data.mMeshType.mEncodedScale / DIVIDER_FLOAT;
        break;

        case eArtResource_AnimatingMesh:
            artResource.mAnimationDesc.mFps = packed_data.mAnimType.mFps;
            artResource.mAnimationDesc.mFrames = packed_data.mAnimType.mFrames;
            artResource.mAnimationDesc.mDistStart = packed_data.mAnimType.mStartDist;
            artResource.mAnimationDesc.mDistEnd = packed_data.mAnimType.mEndDist;
        break;

        case eArtResource_ProceduralMesh:
            artResource.mProcDesc.mId = packed_data.mProcType.mId;
        break;
    }

    return true;
}

bool DK2ScenarioReader::ReadTerrainFlags(TerrainDefinition& terrainDef)
{
    unsigned int terrainFlags = 0;
    READ_FSTREAM_DWORD(mFileStream, terrainFlags);

    terrainDef.mIsSolid = (terrainFlags & TERRAINF_SOLID) > 0;
    terrainDef.mIsImpenetrable = (terrainFlags & TERRAINF_IMPENETRABLE) > 0;
    terrainDef.mIsOwnable = (terrainFlags & TERRAINF_OWNABLE) > 0;
    terrainDef.mIsTaggable = (terrainFlags & TERRAINF_TAGGABLE) > 0;
    terrainDef.mIsAttackable = (terrainFlags & TERRAINF_ATTACKABLE) > 0;
    terrainDef.mHasTorch = (terrainFlags & TERRAINF_TORCH) > 0;
    terrainDef.mIsWater = (terrainFlags & TERRAINF_WATER) > 0;
    terrainDef.mIsLava = (terrainFlags & TERRAINF_LAVA) > 0;
    terrainDef.mAlwaysExplored = (terrainFlags & TERRAINF_ALWAYS_EXPLORED) > 0;
    terrainDef.mPlayerColouredPath = (terrainFlags & TERRAINF_PLAYER_COLOURED_PATH) > 0;
    terrainDef.mPlayerColouredWall = (terrainFlags & TERRAINF_PLAYER_COLOURED_WALL) > 0;
    terrainDef.mConstructionTypeWater = (terrainFlags & TERRAINF_CONSTRUCTION_TYPE_WATER) > 0;
    terrainDef.mConstructionTypeQuad = (terrainFlags & TERRAINF_CONSTRUCTION_TYPE_QUAD) > 0;
    terrainDef.mUnexploreIfDugByAnotherPlayer = (terrainFlags & TERRAINF_UNEXPLORE_IF_DUG_BY_ANOTHER_PLAYER) > 0;
    terrainDef.mFillInable = (terrainFlags & TERRAINF_FILL_INABLE) > 0;
    terrainDef.mAllowRoomWalls = (terrainFlags & TERRAINF_ALLOW_ROOM_WALLS) > 0;
    terrainDef.mIsDecay = (terrainFlags & TERRAINF_DECAY) > 0;
    terrainDef.mHasRandomTexture = (terrainFlags & TERRAINF_RANDOM_TEXTURE) > 0;
    terrainDef.mTerrainColorR = (terrainFlags & TERRAINF_TERRAIN_COLOR_R) > 0;
    terrainDef.mTerrainColorG = (terrainFlags & TERRAINF_TERRAIN_COLOR_G) > 0;
    terrainDef.mTerrainColorB = (terrainFlags & TERRAINF_TERRAIN_COLOR_B) > 0;
    terrainDef.mDwarfCanDigThrough = (terrainFlags & TERRAINF_DWARF_CAN_DIG_THROUGH) > 0;
    terrainDef.mRevealThroughFogOfWar = (terrainFlags & TERRAINF_REVEAL_THROUGH_FOG_OF_WAR) > 0;
    terrainDef.mAmbientColorR = (terrainFlags & TERRAINF_AMBIENT_COLOR_R) > 0;
    terrainDef.mAmbientColorG = (terrainFlags & TERRAINF_AMBIENT_COLOR_G) > 0;
    terrainDef.mAmbientColorB = (terrainFlags & TERRAINF_AMBIENT_COLOR_B) > 0;
    terrainDef.mHasLight = (terrainFlags & TERRAINF_LIGHT) > 0;
    terrainDef.mHasAmbientLight = (terrainFlags & TERRAINF_AMBIENT_LIGHT) > 0;

    return true;
}

bool DK2ScenarioReader::ReadRoomFlags(RoomDefinition& roomDef)
{
    unsigned int roomFlags = 0;
    READ_FSTREAM_DWORD(mFileStream, roomFlags);

    roomDef.mPlaceableOnWater = (roomFlags & ROOMF_PLACEABLE_ON_WATER) > 0;
    roomDef.mPlaceableOnLava = (roomFlags & ROOMF_PLACEABLE_ON_LAVA) > 0;
    roomDef.mPlaceableOnLand = (roomFlags & ROOMF_PLACEABLE_ON_LAND) > 0;
    roomDef.mHasWalls = (roomFlags & ROOMF_HAS_WALLS) > 0;
    roomDef.mCentre = (roomFlags & ROOMF_CENTRE) > 0;
    roomDef.mSpecialTiles = (roomFlags & ROOMF_SPECIAL_TILES) > 0;
    roomDef.mNormalTiles = (roomFlags & ROOMF_NORMAL_TILES) > 0;
    roomDef.mBuildable = (roomFlags & ROOMF_BUILDABLE) > 0;
    roomDef.mSpecialWalls = (roomFlags & ROOMF_SPECIAL_WALLS) > 0;
    roomDef.mIsAttackable = (roomFlags & ROOMF_ATTACKABLE) > 0;
    roomDef.mHasFlame = (roomFlags & ROOMF_HAS_FLAME) > 0;
    roomDef.mIsGood = (roomFlags & ROOMF_IS_GOOD) > 0;

    return true;
}

bool DK2ScenarioReader::ReadObjectFlags(GameObjectDefinition& objectDef)
{
    unsigned int objectFlags;
    READ_FSTREAM_DWORD(mFileStream, objectFlags);

    objectDef.mDieOverTime = (objectFlags & OBJECTF_DIE_OVER_TIME) > 0;
    objectDef.mDieOverTimeIfNotInRoom = (objectFlags & OBJECTF_DIE_OVER_TIME_IF_NOT_IN_ROOM) > 0;
    objectDef.mCanBePickedUp = (objectFlags & OBJECTF_CAN_BE_PICKED_UP) > 0;
    objectDef.mCanBeSlapped = (objectFlags & OBJECTF_CAN_BE_SLAPPED) > 0;
    objectDef.mDieWhenSlapped = (objectFlags & OBJECTF_DIE_WHEN_SLAPPED) > 0;
    objectDef.mCanBeDroppedOnAnyLand = (objectFlags & OBJECTF_CAN_BE_DROPPED_ON_ANY_LAND) > 0;
    objectDef.mObstacle = (objectFlags & OBJECTF_OBSTACLE) > 0;
    objectDef.mBounce = (objectFlags & OBJECTF_BOUNCE) > 0;
    objectDef.mBoulderCanRollThrough = (objectFlags & OBJECTF_BOULDER_CAN_ROLL_THROUGH) > 0;
    objectDef.mBoulderDestroys = (objectFlags & OBJECTF_BOULDER_DESTROYS) > 0;
    objectDef.mIsPillar = (objectFlags & OBJECTF_PILLAR) > 0;
    objectDef.mDoorKey = (objectFlags & OBJECTF_DOOR_KEY) > 0;
    objectDef.mIsDamageable = (objectFlags & OBJECTF_DAMAGEABLE) > 0;
    objectDef.mHighlightable = (objectFlags & OBJECTF_HIGHLIGHTABLE) > 0;
    objectDef.mPlaceable = (objectFlags & OBJECTF_PLACEABLE) > 0;
    objectDef.mFirstPersonObstacle = (objectFlags & OBJECTF_FIRST_PERSON_OBSTACLE) > 0;
    objectDef.mSolidObstacle = (objectFlags & OBJECTF_SOLID_OBSTACLE) > 0;
    objectDef.mCastShadows = (objectFlags & OBJECTF_CAST_SHADOWS) > 0;

    // category
    objectDef.mObjectCategory = eGameObjectCategory_Normal;
    {
        const unsigned int categoryBitsMask = OBJECTF_TYPE_SPECIAL | OBJECTF_TYPE_SPELL_BOOK | OBJECTF_TYPE_CRATE | 
            OBJECTF_TYPE_LAIR | OBJECTF_TYPE_GOLD | OBJECTF_TYPE_FOOD | OBJECTF_TYPE_LEVEL_GEM;
        switch (objectFlags & categoryBitsMask)
        {
            case OBJECTF_TYPE_SPECIAL   : objectDef.mObjectCategory = eGameObjectCategory_Special; break;
            case OBJECTF_TYPE_SPELL_BOOK: objectDef.mObjectCategory = eGameObjectCategory_SpellBook; break;
            case OBJECTF_TYPE_CRATE     : objectDef.mObjectCategory = eGameObjectCategory_Crate; break;
            case OBJECTF_TYPE_LAIR      : objectDef.mObjectCategory = eGameObjectCategory_Lair; break;
            case OBJECTF_TYPE_GOLD      : objectDef.mObjectCategory = eGameObjectCategory_Gold; break;
            case OBJECTF_TYPE_FOOD      : objectDef.mObjectCategory = eGameObjectCategory_Food; break;
            case OBJECTF_TYPE_LEVEL_GEM : objectDef.mObjectCategory = eGameObjectCategory_LevelGem; break;
            default:
                cxx_assert((objectFlags & categoryBitsMask) == 0);
            break;
        }
    }

    return true;
}

bool DK2ScenarioReader::ReadStringId()
{
    unsigned int ids[5];
    for (unsigned int& idEntry : ids)
    {
        READ_FROM_FSTREAM(mFileStream, idEntry);
    }

    unsigned int unknownDword;
    READ_FROM_FSTREAM(mFileStream, unknownDword);

    return true;
}

bool DK2ScenarioReader::ReadMapData(ScenarioDefinition& scenarioData)
{
    scenarioData.mMapTiles.resize(scenarioData.mLevelDimensionX * scenarioData.mLevelDimensionY);

    // read tiles
    for (int tiley = 0; tiley < scenarioData.mLevelDimensionY; ++tiley)
    for (int tilex = 0; tilex < scenarioData.mLevelDimensionX; ++tilex)
    {
        const int tileIndex = (tiley * scenarioData.mLevelDimensionX) + tilex;

        // terrain type is not mapped to internal id so it can be red as is
        READ_FSTREAM_BYTE(mFileStream, scenarioData.mMapTiles[tileIndex].mTerrainType);

        unsigned char playerID;
        READ_FSTREAM_BYTE(mFileStream, playerID);
        if (!KWDIntToENUM(playerID, scenarioData.mMapTiles[tileIndex].mOwnerID))
            return false;

        unsigned char bridgeTerrain;
        READ_FSTREAM_BYTE(mFileStream, bridgeTerrain);
        if (!KWDIntToENUM(bridgeTerrain, scenarioData.mMapTiles[tileIndex].mTerrainUnderTheBridge))
            return false;

        unsigned char filler;
        READ_FROM_FSTREAM(mFileStream, filler);
    }

    return true;
}

bool DK2ScenarioReader::ReadScenarioVariables(int numElements, ScenarioDefinition& scenarioData)
{
    for (int ielement = 0; ielement < numElements; ++ielement)
    {
        int variableType = 0;
        READ_FROM_FSTREAM(mFileStream, variableType);

        if (variableType == ScenarioVariableType_CreaturePool)
        {
            int dummyInt;
            READ_FROM_FSTREAM(mFileStream, dummyInt); // creature id
            READ_FROM_FSTREAM(mFileStream, dummyInt); // value
            READ_FROM_FSTREAM(mFileStream, dummyInt); // player id
            continue;
        }

        if (variableType == ScenarioVariableType_Availability)
        {
            unsigned short dummyWord;
            READ_FROM_FSTREAM(mFileStream, dummyWord); // availability type
            READ_FROM_FSTREAM(mFileStream, dummyWord); // player id
            int dummyInt;
            READ_FROM_FSTREAM(mFileStream, dummyInt); // type id
            READ_FROM_FSTREAM(mFileStream, dummyInt); // availability value
            continue;
        }

        if (variableType == ScenarioVariableType_SacrificesId)
        {
            unsigned char dummyByte;
            READ_FROM_FSTREAM(mFileStream, dummyByte); // type 1
            READ_FROM_FSTREAM(mFileStream, dummyByte); // id 1
            READ_FROM_FSTREAM(mFileStream, dummyByte); // type 2
            READ_FROM_FSTREAM(mFileStream, dummyByte); // id 2
            READ_FROM_FSTREAM(mFileStream, dummyByte); // type 3
            READ_FROM_FSTREAM(mFileStream, dummyByte); // id 3
            READ_FROM_FSTREAM(mFileStream, dummyByte); // sacrifice reward type
            READ_FROM_FSTREAM(mFileStream, dummyByte); // speech id
            int dummyInt;
            READ_FROM_FSTREAM(mFileStream, dummyInt); // reward value
            continue;
        }

        if ((variableType == ScenarioVariableType_CreatureStatsId) ||
            (variableType == ScenarioVariableType_CreatureFirstPersonId))
        {
            int dummyInt;
            READ_FROM_FSTREAM(mFileStream, dummyInt); // stat type
            READ_FROM_FSTREAM(mFileStream, dummyInt); // value
            READ_FROM_FSTREAM(mFileStream, dummyInt); // level
            continue;
        }

        // common
        int intValue;
        READ_FROM_FSTREAM(mFileStream, intValue); // value

        int dummyInt;
        READ_FROM_FSTREAM(mFileStream, dummyInt); // unknown 1
        READ_FROM_FSTREAM(mFileStream, dummyInt); // unknown 1

        switch (variableType)
        {
            case ScenarioVariableType_GoldMinedFromGems:
                scenarioData.mVariables.mGoldMinedFromGems = intValue;
            break;
            case ScenarioVariableType_MaxGoldPerTreasuryTile:
                scenarioData.mVariables.mMaxGoldPerTreasuryTile = intValue;
            break;
            case ScenarioVariableType_SpecialIncreaseGoldAmount:
                scenarioData.mVariables.mSpecialIncreaseGoldAmount = intValue;
            break;
            case ScenarioVariableType_MaxGoldPileOutsideTreasury:
                scenarioData.mVariables.mMaxGoldPileOutsideTreasury = intValue;
            break;
            case ScenarioVariableType_MaxGoldPerDungeonHeartTile:
                scenarioData.mVariables.mMaxGoldPerDungeonHeartTile = intValue;
            break;
            case ScenarioVariableType_MaximumManaThreshold:
                scenarioData.mVariables.mMaximumManaThreshold = intValue;
            break;
        }
    }

    return true;
}

bool DK2ScenarioReader::ReadObjectDefinition(GameObjectDefinition& objectDef)
{
    if (!ReadString8(32, objectDef.mObjectName))
        return false;

    // resources
    if (!ReadArtResource(objectDef.mResourceMesh))
        return false;

    if (!ReadArtResource(objectDef.mResourceGuiIcon))
        return false;

    if (!ReadArtResource(objectDef.mResourceInHandIcon))
        return false;

    if (!ReadArtResource(objectDef.mResourceInHandMesh))
        return false;

    if (!ReadArtResource(objectDef.mResourceUnknown))
        return false;

    if (!ReadArtResource(objectDef.mResourceAdditional1))
        return false;

    if (!ReadArtResource(objectDef.mResourceAdditional2))
        return false;

    if (!ReadArtResource(objectDef.mResourceAdditional3))
        return false;

    if (!ReadArtResource(objectDef.mResourceAdditional4))
        return false;

    if (!ReadLight())
        return false;

    unsigned int width;
    unsigned int height;
    unsigned int mass;
    unsigned int speed;
    unsigned int airFriction;

    READ_FROM_FSTREAM(mFileStream, width);
    READ_FROM_FSTREAM(mFileStream, height);
    READ_FROM_FSTREAM(mFileStream, mass); 
    READ_FROM_FSTREAM(mFileStream, speed);
    READ_FROM_FSTREAM(mFileStream, airFriction);

    // set params
    objectDef.mWidth = (width * 1.0f) / DIVIDER_FLOAT;
    objectDef.mHeight = (height * 1.0f) / DIVIDER_FLOAT;
    objectDef.mMass = (mass * 1.0f) / DIVIDER_FLOAT;
    objectDef.mSpeed = (speed * 1.0f) / DIVIDER_FLOAT;
    objectDef.mAirFriction = (airFriction * 1.0f) / DIVIDER_DOUBLE;

    unsigned char objMaterial;
    READ_FSTREAM_BYTE(mFileStream, objMaterial);
    if (!KWDIntToENUM(objMaterial, objectDef.mObjectMaterial))
        return false;

    SKIP_FSTREAM_BYTES(mFileStream, 3);

    if (!ReadObjectFlags(objectDef))
        return false;

    READ_FSTREAM_WORD(mFileStream, objectDef.mHitpoints);
    READ_FSTREAM_WORD(mFileStream, objectDef.mMaxAngle);
    SKIP_FSTREAM_BYTES(mFileStream, 2);
    READ_FSTREAM_WORD(mFileStream, objectDef.mManaValue);
    READ_FSTREAM_WORD(mFileStream, objectDef.mTooltipStringId);
    READ_FSTREAM_WORD(mFileStream, objectDef.mNameStringId);
    READ_FSTREAM_WORD(mFileStream, objectDef.mSlapEffectId);
    READ_FSTREAM_WORD(mFileStream, objectDef.mDeathEffectId);
    READ_FSTREAM_WORD(mFileStream, objectDef.mMiscEffectId);

    // object type is not mapped to internal id so it can be red as is
    READ_FSTREAM_BYTE(mFileStream, objectDef.mObjectClass);

    unsigned char initialState;
    READ_FROM_FSTREAM(mFileStream, initialState);

    cxx_assert(initialState < eGameObjectState_MAX);
    objectDef.mStartState = static_cast<eGameObjectState>(initialState);

    READ_FSTREAM_BYTE(mFileStream, objectDef.mRoomCapacity);

    unsigned char pickupPriority;
    READ_FROM_FSTREAM(mFileStream, pickupPriority);

    // sound category
    if (!ReadString8(32, objectDef.mSoundCategory))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadObjectsData(int numElements, ScenarioDefinition& scenarioData)
{
    scenarioData.mGameObjectDefs.resize(numElements + 1);
    scenarioData.mGameObjectDefs[0] = {}; // dummy element

    // read definitions
    for (int iobject = 1; iobject < numElements + 1; ++iobject)
    {
        if (!ReadObjectDefinition(scenarioData.mGameObjectDefs[iobject]))
            return false;

        bool correctId = (scenarioData.mGameObjectDefs[iobject].mObjectClass == iobject);
        cxx_assert(correctId);
    }
    return true;
}

bool DK2ScenarioReader::ReadPlayerDefinition(PlayerDefinition& playerDef)
{
    READ_FSTREAM_DWORD(mFileStream, playerDef.mInitialGold);

    unsigned int playerType;
    READ_FSTREAM_DWORD(mFileStream, playerType);
    if (!KWDIntToENUM(playerType, playerDef.mPlayerType))
        return false;

    unsigned char aiType;
    READ_FSTREAM_BYTE(mFileStream, aiType);
    if (!KWDIntToENUM(aiType, playerDef.mComputerAI))
        return false;

    unsigned char speed;
    READ_FROM_FSTREAM(mFileStream, speed);

    unsigned char openness;
    READ_FROM_FSTREAM(mFileStream, openness);

    unsigned char fillerByte;
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    unsigned int fillerDword;
    READ_FROM_FSTREAM(mFileStream, fillerDword);
    READ_FROM_FSTREAM(mFileStream, fillerDword);
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerByte);

    // build order
    unsigned char buildOrderBytes[15];
    if (!mFileStream.read(reinterpret_cast<char*>(buildOrderBytes), sizeof(buildOrderBytes)))
        return false;

    READ_FROM_FSTREAM(mFileStream, fillerByte); // flexibility
    READ_FROM_FSTREAM(mFileStream, fillerByte); // digToNeutralRoomsWithinTilesOfClaimedArea
    
    unsigned short fillerWord;
    READ_FROM_FSTREAM(mFileStream, fillerWord); // removeCallToArmsAfterSeconds

    READ_FROM_FSTREAM(mFileStream, fillerDword); // boulderTrapsOnLongCorridors
    READ_FROM_FSTREAM(mFileStream, fillerDword); // boulderTrapsOnRouteToBreachPoints

    READ_FROM_FSTREAM(mFileStream, fillerByte); // trapUseStyle
    READ_FROM_FSTREAM(mFileStream, fillerByte); // doorTrapPreference

    READ_FROM_FSTREAM(mFileStream, fillerByte); // doorUsage
    READ_FROM_FSTREAM(mFileStream, fillerByte); // chanceOfLookingToUseTrapsAndDoors

    READ_FROM_FSTREAM(mFileStream, fillerDword); // requireMinLevelForCreatures
    READ_FROM_FSTREAM(mFileStream, fillerDword); // requireTotalThreatGreaterThanTheEnemy
    READ_FROM_FSTREAM(mFileStream, fillerDword); // requireAllRoomTypesPlaced
    READ_FROM_FSTREAM(mFileStream, fillerDword); // requireAllKeeperSpellsResearched
    READ_FROM_FSTREAM(mFileStream, fillerDword); // onlyAttackAttackers
    READ_FROM_FSTREAM(mFileStream, fillerDword); // neverAttack

    READ_FROM_FSTREAM(mFileStream, fillerByte); // minLevelForCreatures
    READ_FROM_FSTREAM(mFileStream, fillerByte); // totalThreatGreaterThanTheEnemy
    READ_FROM_FSTREAM(mFileStream, fillerByte); // firstAttemptToBreachRoom
    READ_FROM_FSTREAM(mFileStream, fillerByte); // firstDigToEnemyPoint
    READ_FROM_FSTREAM(mFileStream, fillerByte); // breachAtPointsSimultaneously
    READ_FROM_FSTREAM(mFileStream, fillerByte); // usePercentageOfTotalCreaturesInFirstFightAfterBreach

    READ_FSTREAM_WORD(mFileStream, playerDef.mManaValue); 
    READ_FROM_FSTREAM(mFileStream, fillerWord); // placeCallToArmsWhereThreatValueIsGreaterThan
    READ_FROM_FSTREAM(mFileStream, fillerWord); // removeCallToArmsIfLessThanEnemyCreatures

    READ_FROM_FSTREAM(mFileStream, fillerWord); // removeCallToArmsIfLessThanEnemyCreaturesWithinTiles
    READ_FROM_FSTREAM(mFileStream, fillerWord); // pullCreaturesFromFightIfOutnumberedAndUnableToDropReinforcements

    READ_FROM_FSTREAM(mFileStream, fillerByte); // threatValueOfDroppedCreaturesIsPercentageOfEnemyThreatValue
    READ_FROM_FSTREAM(mFileStream, fillerByte); // spellStyle
    READ_FROM_FSTREAM(mFileStream, fillerByte); // attemptToImprisonPercentageOfEnemyCreatures
    READ_FROM_FSTREAM(mFileStream, fillerByte); // ifCreatureHealthIsPercentageAndNotInOwnRoomMoveToLairOrTemple

    READ_FSTREAM_WORD(mFileStream, playerDef.mGoldValue);

    READ_FROM_FSTREAM(mFileStream, fillerDword); // tryToMakeUnhappyOnesHappy
    READ_FROM_FSTREAM(mFileStream, fillerDword); // tryToMakeAngryOnesHappy
    READ_FROM_FSTREAM(mFileStream, fillerDword); // disposeOfAngryCreatures
    READ_FROM_FSTREAM(mFileStream, fillerDword); // disposeOfRubbishCreaturesIfBetterOnesComeAlong

    READ_FROM_FSTREAM(mFileStream, fillerByte); // disposalMethod
    READ_FROM_FSTREAM(mFileStream, fillerByte); // maximumNumberOfImps
    READ_FROM_FSTREAM(mFileStream, fillerByte); // willNotSlapCreatures
    READ_FROM_FSTREAM(mFileStream, fillerByte); // attackWhenNumberOfCreaturesIsAtLeast

    READ_FROM_FSTREAM(mFileStream, fillerDword); // useLightningIfEnemyIsInWater

    READ_FROM_FSTREAM(mFileStream, fillerByte); // useSightOfEvil
    READ_FROM_FSTREAM(mFileStream, fillerByte); // useSpellsInBattle
    READ_FROM_FSTREAM(mFileStream, fillerByte); // spellsPowerPreference
    READ_FROM_FSTREAM(mFileStream, fillerByte); // useCallToArms
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerWord); // mineGoldUntil
    READ_FROM_FSTREAM(mFileStream, fillerWord); // waitSecondsAfterPreviousAttackBeforeAttackingAgain

    READ_FSTREAM_DWORD(mFileStream, playerDef.mInitialMana);

    READ_FROM_FSTREAM(mFileStream, fillerWord); // exploreUpToTilesToFindSpecials
    READ_FROM_FSTREAM(mFileStream, fillerWord); // impTilesRatio
    READ_FROM_FSTREAM(mFileStream, fillerWord); // buildStartX
    READ_FROM_FSTREAM(mFileStream, fillerWord); // buildStartY
    READ_FROM_FSTREAM(mFileStream, fillerWord); // buildEndX
    READ_FROM_FSTREAM(mFileStream, fillerWord); // buildEndY

    READ_FROM_FSTREAM(mFileStream, fillerByte); // likelyhoodToMovingCreaturesToLibraryForResearching
    READ_FROM_FSTREAM(mFileStream, fillerByte); // chanceOfExploringToFindSpecials
    READ_FROM_FSTREAM(mFileStream, fillerByte); // chanceOfFindingSpecialsWhenExploring
    READ_FROM_FSTREAM(mFileStream, fillerByte); // fateOfImprisonedCreatures

    READ_FROM_FSTREAM(mFileStream, fillerWord); // triggerId

    unsigned char playerID;
    READ_FSTREAM_BYTE(mFileStream, playerID);
    if (!KWDIntToENUM(playerID, playerDef.mPlayerIdentifier))
        return false;

    READ_FSTREAM_WORD(mFileStream, playerDef.mStartCameraX); // cameraX
    READ_FSTREAM_WORD(mFileStream, playerDef.mStartCameraY); // cameraY

    if (!ReadString8(32, playerDef.mPlayerName))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadObjectThingData(ScenarioObjectThing& objectThingData)
{
    int posx = 0;
    int posy = 0;
    READ_FROM_FSTREAM(mFileStream, posx);
    READ_FROM_FSTREAM(mFileStream, posy);
    objectThingData.mPositionX = posx;
    objectThingData.mPositionY = posy;

    unsigned int unknownData;
    READ_FROM_FSTREAM(mFileStream, unknownData);

    int keeperSpellId = 0;
    READ_FROM_FSTREAM(mFileStream, keeperSpellId);
    objectThingData.mKeeperSpellId = keeperSpellId;

    int moneyAmount = 0;
    READ_FROM_FSTREAM(mFileStream, moneyAmount);
    objectThingData.mMoneyAmount = moneyAmount;

    unsigned short triggerId = 0;
    READ_FROM_FSTREAM(mFileStream, triggerId);
    objectThingData.mTriggerId = triggerId;

    unsigned char objectId = 0;
    READ_FROM_FSTREAM(mFileStream, objectId);
    objectThingData.mObjectClassId = objectId;

    unsigned char playerId = 0;
    READ_FROM_FSTREAM(mFileStream, playerId);
    if (!KWDIntToENUM(playerId, objectThingData.mPlayerId))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadThingsData(int numElements, ScenarioDefinition& scenarioData)
{
    for (int ielement = 0; ielement < numElements; ++ielement)
    {
        unsigned int thingType = 0;
        READ_FROM_FSTREAM(mFileStream, thingType);
        unsigned int dataSize = 0;
        READ_FROM_FSTREAM(mFileStream, dataSize);

        bool wasSkipped = true;
        switch (thingType)
        {
            case DK_OBJECT_THING:
            {
                wasSkipped = false;
                if (!ReadObjectThingData(scenarioData.mObjectThings.emplace_back()))
                {
                    scenarioData.mObjectThings.pop_back();
                    return false;
                }
            }
            break;
            case DK_TRAP_THING:
            break;
            case DK_DOOR_THING:
            break;
            case DK_ACTIONPOINT_THING:
            break;
            case DK_NEUTRAL_CREATURE_THING:
            break;
            case DK_GOOD_CREATURE_THING:
            break;
            case DK_CREATURE_THING:
            break;
            case DK_HEROPARTY_THING:
            break;
            case DK_DEAD_BODY_THING:
            break;
            case DK_EFFECT_GENERATOR_THING:
            break;
            case DK_ROOM_THING:
            break;
            case DK_CAMERA_THING:
            break;
            default:
                cxx_assert(false);
            break;
        }

        if (wasSkipped)
        {
            SKIP_FSTREAM_BYTES(mFileStream, dataSize);
        }
    }
    return true;
}

bool DK2ScenarioReader::ReadPlayersData(int numElements, ScenarioDefinition& scenarioData)
{
    scenarioData.mPlayerDefs.resize(numElements + 1);
    scenarioData.mPlayerDefs[0] = {}; // dummy element

    // read definitions
    for (int iplayer = 1; iplayer < numElements + 1; ++iplayer)
    {
        if (!ReadPlayerDefinition(scenarioData.mPlayerDefs[iplayer]))
            return false;

        bool correctId = (scenarioData.mPlayerDefs[iplayer].mPlayerIdentifier == iplayer);
        cxx_assert(correctId);
    }
    return true;
}

bool DK2ScenarioReader::ReadRoomDefinition(RoomDefinition& roomDef)
{
    if (!ReadString8(32, roomDef.mRoomName))
        return false;

    // resources
    if (!ReadArtResource(roomDef.mGuiIcon))
        return false;

    if (!ReadArtResource(roomDef.mEditorIcon))
        return false;

    if (!ReadArtResource(roomDef.mCompleteResource))
        return false;

    if (!ReadArtResource(roomDef.mStraightResource))
        return false;

    if (!ReadArtResource(roomDef.mInsideCornerResource))
        return false;

    if (!ReadArtResource(roomDef.mUnknownResource))
        return false;

    if (!ReadArtResource(roomDef.mOutsideCornerResource))
        return false;

    if (!ReadArtResource(roomDef.mWallResource))
        return false;

    if (!ReadArtResource(roomDef.mCapResource))
        return false;

    if (!ReadArtResource(roomDef.mCeilingResource))
        return false;

    unsigned int ceilingHeight;
    READ_FROM_FSTREAM(mFileStream, ceilingHeight);

    unsigned short fillerWord;
    READ_FROM_FSTREAM(mFileStream, fillerWord);

    unsigned short torchIntensity;
    READ_FROM_FSTREAM(mFileStream, torchIntensity);

    if (!ReadRoomFlags(roomDef))
        return false;

    unsigned short tooltipStringId;
    READ_FROM_FSTREAM(mFileStream, tooltipStringId);

    unsigned short nameStringId;
    READ_FROM_FSTREAM(mFileStream, nameStringId);
    READ_FSTREAM_WORD(mFileStream, roomDef.mCost);

    unsigned short fightEffectId;
    READ_FROM_FSTREAM(mFileStream, fightEffectId);

    unsigned short generalDescriptionStringId;
    READ_FROM_FSTREAM(mFileStream, generalDescriptionStringId);

    unsigned short strenghtStringId;
    READ_FROM_FSTREAM(mFileStream, strenghtStringId);

    unsigned short torchRadius;
    READ_FROM_FSTREAM(mFileStream, torchRadius);

    unsigned short effects[8];
    for (unsigned short& effectEntry : effects)
    {
        READ_FROM_FSTREAM(mFileStream, effectEntry);
    }
    READ_FSTREAM_BYTE(mFileStream, roomDef.mRoomType);

    unsigned char fillerByte;
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FSTREAM_BYTE(mFileStream, roomDef.mTerrainType);

    unsigned char tileConstruction;
    READ_FSTREAM_BYTE(mFileStream, tileConstruction);
    if (!KWDIntToENUM(tileConstruction, roomDef.mTileConstruction))
        return false;

    unsigned char createdCreatureId;
    READ_FROM_FSTREAM(mFileStream, createdCreatureId);

    unsigned char torchColor[3];
    for (unsigned char& colorComponent : torchColor)
    {
        READ_FROM_FSTREAM(mFileStream, colorComponent);
    }

    for (GameObjectClassId& objectid : roomDef.mObjectIds)
    {
        READ_FSTREAM_BYTE(mFileStream, objectid);
    }

    if (!ReadString8(32, roomDef.mSoundCategory))
        return false;

    READ_FSTREAM_BYTE(mFileStream, roomDef.mOrderInEditor);
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    if (!ReadArtResource(roomDef.mTorchResource))
        return false;

    READ_FSTREAM_BYTE(mFileStream, roomDef.mRecommendedSizeX);
    READ_FSTREAM_BYTE(mFileStream, roomDef.mRecommendedSizeY);

    short healthGain;
    READ_FROM_FSTREAM(mFileStream, healthGain);

    return true;
}

bool DK2ScenarioReader::ReadRoomsData(int numElements, ScenarioDefinition& scenarioData)
{
    scenarioData.mRoomDefs.resize(numElements + 1);
    scenarioData.mRoomDefs[0] = {}; // dummy element

    // read definitions
    for (int iroom = 1; iroom < numElements + 1; ++iroom)
    {
        if (!ReadRoomDefinition(scenarioData.mRoomDefs[iroom]))
            return false;

        bool correctId = (scenarioData.mRoomDefs[iroom].mRoomType == iroom);
        cxx_assert(correctId);
    }

    return true;
}

bool DK2ScenarioReader::ReadCreaturesDefinition(const KWDFileHeader& header, CreatureDefinition& creature)
{
    std::streamoff startoffset = mFileStream.tellg();
    std::streamoff elementSize = header.mContentSize / header.mItemsCount;
    cxx_assert(elementSize);

    if (!ReadString8(32, creature.mCreatureName))
        return false;

    SKIP_FSTREAM_BYTES(mFileStream, 84); // unknown data
    
    // parse primary animations
    for (int ianim = 0; ianim < 36; ++ianim)
    {
        if (!ReadArtResource(creature.mAnimationResources[ianim]))
            return false;
    }

    mFileStream.seekg(startoffset + elementSize);
    return true;
}

bool DK2ScenarioReader::ReadCreaturesData(const KWDFileHeader& header, ScenarioDefinition& scenarioData) 
{
    scenarioData.mCreatureDefs.resize(header.mItemsCount + 1);
    scenarioData.mCreatureDefs[0] = {}; // dummy element

    std::streamoff startoffset = mFileStream.tellg();
    // read definitions
    for (int icreature = 1; icreature < header.mItemsCount + 1; ++icreature)
    {
        if (!ReadCreaturesDefinition(header, scenarioData.mCreatureDefs[icreature]))
            return false;

//         bool correctId = (scenarioData.mCreatureDefs[icreature].mCreatureClass == icreature);
//         debug_assert(correctId);
    }
    std::streamoff endoffset = mFileStream.tellg();

    cxx_assert(endoffset - startoffset == header.mContentSize);
    return true;
}

bool DK2ScenarioReader::ReadTerrainDefinition(TerrainDefinition& terrainDef)
{
    terrainDef = {};

    if (!ReadString8(32, terrainDef.mName))
        return false;

    // complete resource
    if (!ReadArtResource(terrainDef.mResourceComplete))
        return false;

    // side resource
    if (!ReadArtResource(terrainDef.mResourceSide))
        return false;

    // top resource
    if (!ReadArtResource(terrainDef.mResourceTop))
        return false;

    // tagged top resource
    if (!ReadArtResource(terrainDef.mResourceTagged))
        return false;

    if (!ReadStringId())
        return false;

    unsigned int depthInt;
    READ_FROM_FSTREAM(mFileStream, depthInt);

    if (!Read32bitsFloat(terrainDef.mLightHeight))
        return false;

    if (!ReadTerrainFlags(terrainDef))
        return false;

    READ_FSTREAM_WORD(mFileStream, terrainDef.mDamage);

    unsigned short filledWord;
    READ_FROM_FSTREAM(mFileStream, filledWord);
    READ_FROM_FSTREAM(mFileStream, filledWord);

    READ_FSTREAM_WORD(mFileStream, terrainDef.mGoldCapacity);
    READ_FSTREAM_WORD(mFileStream, terrainDef.mManaGain);
    READ_FSTREAM_WORD(mFileStream, terrainDef.mManaGainMax);

    unsigned short toolTipStringId;
    READ_FROM_FSTREAM(mFileStream, toolTipStringId);

    unsigned short nameStringId;
    READ_FROM_FSTREAM(mFileStream, nameStringId);

    unsigned short maxHealthEffectId;
    READ_FROM_FSTREAM(mFileStream, maxHealthEffectId);

    unsigned short destroyedEffectId;
    READ_FROM_FSTREAM(mFileStream, destroyedEffectId);

    unsigned short generalDescriptionStringId;
    READ_FROM_FSTREAM(mFileStream, generalDescriptionStringId);

    unsigned short strengthStringId;
    READ_FROM_FSTREAM(mFileStream, strengthStringId);

    unsigned short weaknessStringId;
    READ_FROM_FSTREAM(mFileStream, weaknessStringId);

    SKIP_FSTREAM_BYTES(mFileStream, 16 * sizeof(unsigned short)); // unknown

    unsigned char wibbleH;
    READ_FROM_FSTREAM(mFileStream, wibbleH);

    unsigned char leanH[3];
    for (unsigned char& entryLeanH : leanH)
    {
        READ_FROM_FSTREAM(mFileStream, entryLeanH);
    }

    unsigned char wibbleV;
    READ_FROM_FSTREAM(mFileStream, wibbleV);

    unsigned char leanV[3];
    for (unsigned char& entryLeanV : leanV)
    {
        READ_FROM_FSTREAM(mFileStream, entryLeanV);
    }

    READ_FSTREAM_BYTE(mFileStream, terrainDef.mTerrainType);
    READ_FSTREAM_WORD(mFileStream, terrainDef.mHealthInitial);
    READ_FSTREAM_BYTE(mFileStream, terrainDef.mBecomesTerrainTypeWhenMaxHealth);
    READ_FSTREAM_BYTE(mFileStream, terrainDef.mBecomesTerrainTypeWhenDestroyed);

    unsigned char colorComponents[3];
    // terrain color
    READ_FSTREAM_BYTE(mFileStream, colorComponents[0]);
    READ_FSTREAM_BYTE(mFileStream, colorComponents[1]);
    READ_FSTREAM_BYTE(mFileStream, colorComponents[2]);
    terrainDef.mTerrainColor.SetComponentsF(
        (terrainDef.mTerrainColorR) ? ((colorComponents[0] + 256) / 512.0f) : (colorComponents[0] / 256.0f),
        (terrainDef.mTerrainColorG) ? ((colorComponents[1] + 256) / 512.0f) : (colorComponents[1] / 256.0f),
        (terrainDef.mTerrainColorB) ? ((colorComponents[2] + 256) / 512.0f) : (colorComponents[2] / 256.0f),
        1.0f
    );

    READ_FSTREAM_BYTE(mFileStream, terrainDef.mTextureFrames);

    std::string soundCategory;
    if (!ReadString8(32, soundCategory))
        return false;

    READ_FSTREAM_WORD(mFileStream, terrainDef.mHealthMax);

    // ambient color
    READ_FSTREAM_BYTE(mFileStream, colorComponents[0]);
    READ_FSTREAM_BYTE(mFileStream, colorComponents[1]);
    READ_FSTREAM_BYTE(mFileStream, colorComponents[2]);
    terrainDef.mAmbientColor.SetComponentsF(
        (terrainDef.mAmbientColorR) ? ((colorComponents[0] + 256) / 512.0f) : (colorComponents[0] / 256.0f),
        (terrainDef.mAmbientColorG) ? ((colorComponents[1] + 256) / 512.0f) : (colorComponents[1] / 256.0f),
        (terrainDef.mAmbientColorB) ? ((colorComponents[2] + 256) / 512.0f) : (colorComponents[2] / 256.0f),
        1.0f
    );

    std::string soundCategoryFirstPerson;
    if (!ReadString8(32, soundCategoryFirstPerson))
        return false;

    unsigned int fillerDword;
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    return true;
}

bool DK2ScenarioReader::ReadTerrainData(int numElements, ScenarioDefinition& scenarioData)
{
    scenarioData.mTerrainDefs.resize(numElements + 1);
    scenarioData.mTerrainDefs[0] = {}; // dummy element

    // read definitions
    for (int ielement = 1; ielement < numElements + 1; ++ielement)
    {
        if (!ReadTerrainDefinition(scenarioData.mTerrainDefs[ielement]))
            return false;

        bool correctId = (scenarioData.mTerrainDefs[ielement].mTerrainType == ielement);
        cxx_assert(correctId);
    }
    return true;
}

bool DK2ScenarioReader::ReadLevelVariables(ScenarioDefinition& scenarioData)
{
    unsigned short fillerWord;
    unsigned int fillerDword;

    READ_FROM_FSTREAM(mFileStream, fillerWord); // trigger id

    unsigned short ticksPerSecond;
    READ_FROM_FSTREAM(mFileStream, ticksPerSecond);

    scenarioData.mTicksPerSecond = 4.0f; // fixed tick rate

    SKIP_FSTREAM_BYTES(mFileStream, 520); // unknown data

    // read text messages
    std::vector<std::wstring> levelMessages;
    levelMessages.resize(20);
    for (std::wstring& messageEntry: levelMessages)
    {
        if (!ReadString(512, messageEntry))
            return false;
    }

    READ_FROM_FSTREAM(mFileStream, fillerWord); // flags

    std::string speechString;
    if (!ReadString8(32, speechString))
        return false;

    unsigned char talismanPieces;
    READ_FROM_FSTREAM(mFileStream, talismanPieces);
    READ_FROM_FSTREAM(mFileStream, fillerDword);
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    unsigned char soundtrack;
    unsigned char textTableId;
    READ_FROM_FSTREAM(mFileStream, soundtrack);
    READ_FROM_FSTREAM(mFileStream, textTableId);
    READ_FROM_FSTREAM(mFileStream, fillerWord); // textTitleId
    READ_FROM_FSTREAM(mFileStream, fillerWord); // textPlotId
    READ_FROM_FSTREAM(mFileStream, fillerWord); // textDebriefId
    READ_FROM_FSTREAM(mFileStream, fillerWord); // textObjectvId
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord); // speclvlIdx

    // unknown data
    SKIP_FSTREAM_BYTES(mFileStream, 8 * sizeof(unsigned char));
    SKIP_FSTREAM_BYTES(mFileStream, 8 * sizeof(unsigned short));

    // path
    std::string terrainPath;
    if (!ReadString8(32, terrainPath))
        return false;

    unsigned char oneShotHornyLev;
    unsigned char fillerByte;
    READ_FROM_FSTREAM(mFileStream, oneShotHornyLev);
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);

    std::wstring heroName;
    if (!ReadString(32, heroName))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadMapInfo(ScenarioDefinition& scenarioData, std::vector<LevelDataFilePath>& paths)
{
    mFileStream.seekg(20); // end of header

    // additional header data

    unsigned short pathCount;
    unsigned short unknownCount;
    unsigned int fillerDword;

    READ_FROM_FSTREAM(mFileStream, pathCount);
    READ_FROM_FSTREAM(mFileStream, unknownCount);
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    // timestamp 1
    if (!ReadTimestamp())
        return false;

    // timestamp 2
    if (!ReadTimestamp())
        return false;

    READ_FROM_FSTREAM(mFileStream, fillerDword);
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    // property data
    if (!ReadString(64, scenarioData.mLevelName))
        return false;

    if (!ReadString(1024, scenarioData.mLevelDescription))
        return false;

    if (!ReadString(64, scenarioData.mLevelAuthor))
        return false;

    if (!ReadString(64, scenarioData.mLevelEmail))
        return false;

    if (!ReadString(1024, scenarioData.mLevelInformation))
        return false;

    // variables
    if (!ReadLevelVariables(scenarioData))
        return false;

    SKIP_FSTREAM_BYTES(mFileStream, 8); // unknown data

    // read paths
    paths.resize(pathCount);
    for (LevelDataFilePath& pathEntry : paths)
    {
        READ_FSTREAM_DWORD(mFileStream, pathEntry.mId);
        READ_FROM_FSTREAM(mFileStream, fillerDword);

        if (!ReadString8(64, pathEntry.mFilePath))
            return false;
    }
    // unknown data
    SKIP_FSTREAM_BYTES(mFileStream, unknownCount * sizeof(unsigned short));
    return !!mFileStream;
}

bool DK2ScenarioReader::ReadDataFile(LevelDataFileID dataTypeId, ScenarioDefinition& scenarioData)
{
    KWDFileHeader headerData;
    if (dataTypeId == DKLD_GLOBALS)
    {
        // skip global info
        return true;
    }

    // read header
    unsigned int headerId;
    READ_FROM_FSTREAM(mFileStream, headerId);
    if (headerId != dataTypeId) // data type id mistmatch
        return false;

    unsigned int byteSize = 0;
    unsigned int checkOne = 0;
    READ_FSTREAM_DWORD(mFileStream, byteSize);
    READ_FSTREAM_DWORD(mFileStream, headerData.mFileSize);
    READ_FSTREAM_DWORD(mFileStream, checkOne);
    READ_FSTREAM_DWORD(mFileStream, headerData.mHeaderEndOffset);

    cxx_assert(byteSize == sizeof(unsigned int));

    std::streamoff startoffset = mFileStream.tellg();
    switch (dataTypeId)
    {
        case DKLD_MAP:
        {
            READ_FROM_FSTREAM(mFileStream, scenarioData.mLevelDimensionX);
            READ_FROM_FSTREAM(mFileStream, scenarioData.mLevelDimensionY);           
        }
        break;
        case DKLD_TRIGGERS:
        {
            SKIP_FSTREAM_BYTES(mFileStream, 4); //itemcount 1
            SKIP_FSTREAM_BYTES(mFileStream, 4); //itemcount 2
            SKIP_FSTREAM_BYTES(mFileStream, 4); //unknown

            if (!ReadTimestamp()) // created
                return false;
            if (!ReadTimestamp()) // modified
                return false;
        }
        break;
        case DKLD_LEVEL:
        {
            READ_FSTREAM_WORD(mFileStream, headerData.mItemsCount);
            SKIP_FSTREAM_BYTES(mFileStream, 2); // height
            SKIP_FSTREAM_BYTES(mFileStream, 4); // unknown

            if (!ReadTimestamp()) // created
                return false;
            if (!ReadTimestamp()) // modified
                return false;
        }
        break;
        default:
        {
            READ_FSTREAM_DWORD(mFileStream, headerData.mItemsCount);
            SKIP_FSTREAM_BYTES(mFileStream, 4); // unknown

            if (!ReadTimestamp()) // created
                return false;
            if (!ReadTimestamp()) // modified
                return false;
        }
        break;
    }

    unsigned int checkTwo;
    READ_FSTREAM_DWORD(mFileStream, checkTwo);
    READ_FSTREAM_DWORD(mFileStream, headerData.mContentSize);

    // read body
    switch (dataTypeId)
    {
        case DKLD_GLOBALS:
        break;
        case DKLD_MAP:
        {
            if (!ReadMapData(scenarioData))
                return false;
        }
        break;
        case DKLD_TERRAIN:
        {
            if (!ReadTerrainData(headerData.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_ROOMS:
        {
            if (!ReadRoomsData(headerData.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_TRAPS:
        break;
        case DKLD_DOORS:
        break;
        case DKLD_KEEPER_SPELLS:
        break;
        case DKLD_CREATURE_SPELLS:
        break;
        case DKLD_CREATURES:
        {
            if (!ReadCreaturesData(headerData, scenarioData))
                return false;
        }
        break;
        case DKLD_PLAYERS:
        {
            if (!ReadPlayersData(headerData.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_THINGS:
        {
            if (!ReadThingsData(headerData.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_TRIGGERS:
        break;
        case DKLD_LEVEL:
        break;
        case DKLD_VARIABLES:
        {
            if (!ReadScenarioVariables(headerData.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_OBJECTS:
        {
            if (!ReadObjectsData(headerData.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_EFFECT_ELEMENTS:
        break;
        case DKLD_SHOTS:
        break;
        case DKLD_EFFECTS:
        break;
    }

    return true;
}

bool DK2ScenarioReader::ExploreTerrainTypes(ScenarioDefinition& scenarioData) const
{
    // all definitions are loaded at this point, so we should map rooms to terrain types
    bool roomAndTerrainDefinitionsNotNull = !(scenarioData.mRoomDefs.empty() || scenarioData.mTerrainDefs.empty());
    if (!roomAndTerrainDefinitionsNotNull)
        return false;

    scenarioData.mRoomByTerrainType.resize(scenarioData.mTerrainDefs.size(), RoomTypeId_Null);
        
    // explore each room definition
    for (RoomDefinition& roomDefinition : scenarioData.mRoomDefs)
    {
        // null definition is being skipped
        if (roomDefinition.mRoomType == RoomTypeId_Null)
            continue;

        // bind identifier
        scenarioData.mRoomByTerrainType[roomDefinition.mTerrainType] = roomDefinition.mRoomType;
    }

    // find special terrain types
    for (TerrainDefinition& terrainDefinition : scenarioData.mTerrainDefs)
    {
        // null definition is being skipped
        if (terrainDefinition.mTerrainType == TerrainTypeId_Null)
            continue;

        if (terrainDefinition.mIsLava)
        {
            scenarioData.mLavaTerrainType = terrainDefinition.mTerrainType;
        }

        if (terrainDefinition.mIsWater)
        {
            scenarioData.mWaterTerrainType = terrainDefinition.mTerrainType;
        }

        if (terrainDefinition.mPlayerColouredPath && (scenarioData.mPlayerColouredPathTerrainType == TerrainTypeId_Null))
        {
            if (scenarioData.mRoomByTerrainType[terrainDefinition.mTerrainType] == RoomTypeId_Null)
            {
                scenarioData.mPlayerColouredPathTerrainType = terrainDefinition.mTerrainType;
            }
        }

        if (terrainDefinition.mPlayerColouredWall)
        {
            scenarioData.mPlayerColouredWallTerrainType = terrainDefinition.mTerrainType;
        }
    }

    return true;
}

void DK2ScenarioReader::ApplyExtensions(ScenarioDefinition& scenarioData) const
{
    // 1 fix terrain resources

    // hero lair complete resource is missed, seems it originally hardcoded in game exe
    const int HeroLairTerrainIdentifier = 35;

    cxx_assert(HeroLairTerrainIdentifier < scenarioData.mTerrainDefs.size());
    TerrainDefinition& terrainDef = scenarioData.mTerrainDefs[HeroLairTerrainIdentifier];
    if (!terrainDef.mResourceComplete.IsDefined())
    {
        terrainDef.mResourceComplete.mResourceType = eArtResource_TerrainMesh;
        terrainDef.mResourceComplete.mResourceName = "hero_outpost_floor";
    }
    else
    {
        cxx_assert(false);
    }
    // does not use player colors
    terrainDef.mPlayerColouredPath = false;

    // 2 define room pillars

    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_WorkShop))
    {
        definition->mPillarObjectId = GameObjectClassId_WorkshopPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Treasury))
    {
        definition->mPillarObjectId = GameObjectClassId_TreasuryPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Hatchery))
    {
        definition->mPillarObjectId = GameObjectClassId_HatcheryPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Casino))
    {
        definition->mPillarObjectId = GameObjectClassId_CasinoPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_CombatPit))
    {
        definition->mPillarObjectId = GameObjectClassId_PitPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Graveyard))
    {
        definition->mPillarObjectId = GameObjectClassId_GraveyardPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_TortureChamber))
    {
        definition->mPillarObjectId = GameObjectClassId_TorturePillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Temple))
    {
        definition->mPillarObjectId = GameObjectClassId_TempleCandlestick;
    }
}

bool DK2ScenarioReader::ReadScenarioData(const std::string& filePath, ScenarioDefinition& scenarioData)
{
    const std::string levelName = FSGetFileNameWithoutExtension(filePath);

    // open file stream
    mFileStream.open(filePath, std::ios::in | std::ios::binary);
    if (!mFileStream.is_open())
        return false;

    std::vector<LevelDataFilePath> paths;
    if (!ReadMapInfo(scenarioData, paths))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Error reading scenario data info from '%s'", levelName.c_str());
        return false;
    }

    mFileStream.close();

    // override paths
    paths.clear();

    // globals
    paths.emplace_back(DKLD_TERRAIN, "../Terrain");
    paths.emplace_back(DKLD_OBJECTS, "../Objects");
    paths.emplace_back(DKLD_ROOMS, "../Rooms");
    paths.emplace_back(DKLD_CREATURES, "../Creatures");
    paths.emplace_back(DKLD_CREATURE_SPELLS, "../CreatureSpells");
    paths.emplace_back(DKLD_TRAPS, "../Traps");
    paths.emplace_back(DKLD_DOORS, "../Doors");
    paths.emplace_back(DKLD_SHOTS, "../Shots");
    paths.emplace_back(DKLD_KEEPER_SPELLS, "../KeeperSpells");
    paths.emplace_back(DKLD_VARIABLES, "../GlobalVariables.kwd");

    // level data
    paths.emplace_back(DKLD_PLAYERS, levelName + "Players.kld");
    paths.emplace_back(DKLD_MAP, levelName + "Map.kld");
    paths.emplace_back(DKLD_TRIGGERS, levelName + "Triggers.kld");
    paths.emplace_back(DKLD_VARIABLES, levelName + "Variables.kld");
    paths.emplace_back(DKLD_THINGS, levelName + "Things.kld");

    // read data from data files
    for (const LevelDataFilePath& pathEntry: paths)
    {
        std::string dataFilePath;
        if (!gFiles.LocateMapData(pathEntry.mFilePath, dataFilePath))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot locate scenario data file '%s'", pathEntry.mFilePath.c_str());
            continue;
        }

        mFileStream.open(dataFilePath, std::ios::in | std::ios::binary);
        if (mFileStream.is_open())
        {
            if (!ReadDataFile(pathEntry.mId, scenarioData))
            {
                gConsole.LogMessage(eLogLevel_Warning, "Error reading scenario data file '%s'", pathEntry.mFilePath.c_str());
                return false;
            }

            mFileStream.close();
        }
    }

    if (!ExploreTerrainTypes(scenarioData))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Error exploring scenario terrain types");
        return false;
    }

    ApplyExtensions(scenarioData);
    return true;
}

bool DK2ScenarioReader::ReadGlobalData(ScenarioDefinition& scenarioData)
{
    std::vector<LevelDataFilePath> paths;

    paths.emplace_back(DKLD_TERRAIN, "../Terrain");
    paths.emplace_back(DKLD_OBJECTS, "../Objects");
    paths.emplace_back(DKLD_ROOMS, "../Rooms");
    paths.emplace_back(DKLD_CREATURES, "../Creatures");
    paths.emplace_back(DKLD_CREATURE_SPELLS, "../CreatureSpells");
    paths.emplace_back(DKLD_TRAPS, "../Traps");
    paths.emplace_back(DKLD_DOORS, "../Doors");
    paths.emplace_back(DKLD_SHOTS, "../Shots");
    paths.emplace_back(DKLD_KEEPER_SPELLS, "../KeeperSpells");
    paths.emplace_back(DKLD_VARIABLES, "../GlobalVariables.kwd");

    // read data from data files
    for (const LevelDataFilePath& pathEntry: paths)
    {
        std::string dataFilePath;
        if (!gFiles.LocateMapData(pathEntry.mFilePath, dataFilePath))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot locate scenario data file '%s'", pathEntry.mFilePath.c_str());
            continue;
        }

        mFileStream.open(dataFilePath, std::ios::in | std::ios::binary);
        if (mFileStream.is_open())
        {
            if (!ReadDataFile(pathEntry.mId, scenarioData))
            {
                gConsole.LogMessage(eLogLevel_Warning, "Error reading scenario data file '%s'", pathEntry.mFilePath.c_str());
                return false;
            }

            mFileStream.close();
        }
    }

    if (!ExploreTerrainTypes(scenarioData))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Error exploring scenario terrain types");
        return false;
    }

    ApplyExtensions(scenarioData);
    return true;
}
