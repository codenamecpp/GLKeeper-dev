#pragma once

//////////////////////////////////////////////////////////////////////////

struct GameObjectCapabilities
{
public:

    //////////////////////////////////////////////////////////////////////////

    class GoldContainer
    {
    public:
        virtual ~GoldContainer() {}
        virtual long GetStoredGoldAmount() const = 0;
        virtual long GetStoredGoldCapacity() const = 0; // returns 0 if capacity is unlimited

        // changes amount of gold in the container
        // typically not used directly by anyone other than the Parent Room Controller,
        // as it may cause resource synchronization issues
        virtual long StoreGold(long goldAmount) = 0; // returns amount of gold actually stored
        virtual long DisposeGold(long goldAmount) = 0; // returns amount of gold actually disposed
    };

    //////////////////////////////////////////////////////////////////////////

public:

    GoldContainer* mGoldContainer = nullptr;
};

//////////////////////////////////////////////////////////////////////////