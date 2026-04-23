#pragma once

#include "CreatureDefs.h"
#include "GameDefs.h"
#include "ScenarioDefs.h"

// Defines generic creature
class Creature
{
public:
    // public for convenience, don't change these fields directly
    CreatureDefinition* mDefinition; // never changes

public:
    // @param creatureDefinition: cannot be null
    Creature(CreatureDefinition* creatureDefinition);
    ~Creature();

private:
};