#pragma once

//////////////////////////////////////////////////////////////////////////

struct RoomCapabilities
{
public:

    //////////////////////////////////////////////////////////////////////////

    class MoneyStorage
    {
    public:
        virtual ~MoneyStorage() {}
        virtual long GetStoredGoldAmount() const = 0;
        virtual long GetStoredGoldCapacity() const = 0; // returns 0 if capacity is unlimited
        virtual long StoreGold(long goldAmount) = 0; // returns amount of gold actually stored
        virtual long DisposeGold(long goldAmount) = 0; // returns amount of gold actually disposed
    };

    //////////////////////////////////////////////////////////////////////////

    class ObjectStorage
    {
    public:
        virtual ~ObjectStorage() {}
        virtual int GetStoredObjectsCount() = 0;
        virtual int GetStoredObjectsCapacity() = 0;
    };

    //////////////////////////////////////////////////////////////////////////

public:

    MoneyStorage* mMoneyStorage = nullptr;
    ObjectStorage* mObjectStorage = nullptr;
};

//////////////////////////////////////////////////////////////////////////