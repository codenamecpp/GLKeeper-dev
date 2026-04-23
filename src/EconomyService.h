#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "PlayerDefs.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class EconomyService: public GameSessionAware
{
public:

    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdateLogicTick(float stepDeltaTime);

    // player resource management
    long GiveResource(ePlayerID playerId, eGameResource resourceType, long resourceAmount);
    long TakeResource(ePlayerID playerId, eGameResource resourceType, long resourceAmount);
    long GiveResource(Player& player, eGameResource resourceType, long resourceAmount);
    long TakeResource(Player& player, eGameResource resourceType, long resourceAmount);

    // notifications
    void StoredGoldAmountChanged(ePlayerID playerId, EntityHandle roomHandle, long amountDelta);

private:
    void IssueStartingResources();


private:
};

//////////////////////////////////////////////////////////////////////////