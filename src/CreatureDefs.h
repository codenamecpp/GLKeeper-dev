#pragma once

enum CreatureFightStyle
{
    CreatureFightStyle_NonFighter,

    // Blitzers are ideally suited to offensive strikes. When confronted with enemy troops, 
    // Blitzers storm towards the front line in an attempt to break through and reach the 
    // enemy support creatures. Once the enemy support has been disabled, they turn their 
    // attentions back to the other creatures.
    CreatureFightStyle_Blitzer, // Dark Angel, Mistress, Skeleton, Vampire

    // Support creatures are generally unsuited to physical, hand-to-hand combat, but their ranged
    // attacks are ideally suited to providing cover and support to the Blitzers and Flankers.
    CreatureFightStyle_Support, // Dark Elf, Warlock, Elven Archer, Wizard

    // Blockers are ideal for defending vital areas of your dungeon. In combat situations, 
    // they don’t storm in on the offensive, but rather hold their position in an attempt 
    // to prevent enemy troops from gaining ground.
    CreatureFightStyle_Blocker, // Bile Demon, Black Knight, Maiden, Troll, Giant, Guard, Knight, Royal Guard
                
    // Flankers attempt to position themselves behind the enemy targets, where they can then 
    // let rip and inflict the maximum damage possible.
    CreatureFightStyle_Flanker
};

enum CreatureJobClass
{
    // Thinker is the job class of Warlocks, Vampires, Maidens, Monks, and Wizards. 
    // It indicates that these creatures' first priority (after food, wages, and sleep) is going to a Library 
    // and researching spells until there is nothing left to research.
    CreatureJobClass_Thinker,

    // Fighter is the job class of most of the creatures - that is to say, every creature that is not a Thinker, 
    // a Worker, or a Scout. It indicates that fighting is what these creatures do best, even if they take it 
    // upon themselves to do other things when the need arises; for example, Dark Angels will research if there 
    // is still research to be done, and Bile Demons and Giants will usually manufacture Traps and Doors if there 
    // are any unfilled blueprints, but all of these creatures are still classified as Fighters.
    CreatureJobClass_Fighter,

    // Worker
    CreatureJobClass_Worker,

    // Scout
    CreatureJobClass_Scout,
};

// All creature classes
enum CreatureClassID
{
    CreatureClassID_Null, // not valid class id
    // todo
    CreatureClassID_COUNT 
};

enum CreatureAnimationID
{
    // primary animations count 36
    CreatureAnimation_Walk,
	CreatureAnimation_Run,
    CreatureAnimation_Dragged_Pose,
    CreatureAnimation_Recoil_Forwards,
    CreatureAnimation_Melee,
    CreatureAnimation_Magic,
    CreatureAnimation_Die,
    CreatureAnimation_Happy,
    CreatureAnimation_Angry,
    CreatureAnimation_Stunned_Pose,
    CreatureAnimation_Swing,
    CreatureAnimation_Sleep_Pose,
    CreatureAnimation_Eat,
    CreatureAnimation_Research,
    CreatureAnimation_Null_NotUsed1,
    CreatureAnimation_Null_NotUsed2,
    CreatureAnimation_Torture,
    CreatureAnimation_Null_NotUsed3,
    CreatureAnimation_Drink,
    CreatureAnimation_Idle1,
    CreatureAnimation_Recoil_Backwards,
    CreatureAnimation_Building,
    CreatureAnimation_Pray,
    CreatureAnimation_Fallback,
    CreatureAnimation_Elec,
    CreatureAnimation_Electrocute,
    CreatureAnimation_Getup,
    CreatureAnimation_Dance,
    CreatureAnimation_Drunk1,
    CreatureAnimation_Entrance,
    CreatureAnimation_Idle2,
    CreatureAnimation_Special1,
    CreatureAnimation_Special2,
    CreatureAnimation_Drunk2,
    CreatureAnimation_Special3,
    CreatureAnimation_Null_NotUsed4,

    // additional animations
    CreatureAnimation_DrunkIdle,
    CreatureAnimation_Melee2,
    CreatureAnimation_Special4,
    CreatureAnimation_Special5,
    CreatureAnimation_Special6,
    CreatureAnimation_Special7,
    CreatureAnimation_Special8,
    CreatureAnimation_WalkBack,
    CreatureAnimation_Pose_Frame,
    CreatureAnimation_Walk2,
    CreatureAnimation_Death_Pose,

    CreatureAnimation_COUNT,

    // special animations, remapped
    CreatureAnimation_Horny_GemIntro = 2,
    CreatureAnimation_Horny_Footstamp = 5,
    CreatureAnimation_Horny_Melee2 = 12,
    CreatureAnimation_Horny_Melee3 = 13,
    CreatureAnimation_Horny_Roar1 = 19,
    CreatureAnimation_Horny_PickUpGem = 22,
    CreatureAnimation_Horny_StampOnChicken = 24,
    CreatureAnimation_Horny_Roar2 = CreatureAnimation_Special3,
    CreatureAnimation_Imp_Dig = 4,
    CreatureAnimation_Imp_ClaimLand = 11,
    CreatureAnimation_Imp_ClaimFloor = 12,
    CreatureAnimation_Imp_Drag = 18,
    CreatureAnimation_Imp_Jump1 = CreatureAnimation_Special1,
    CreatureAnimation_Imp_Jump2 = CreatureAnimation_Special3,
    CreatureAnimation_Imp_Idle3 = CreatureAnimation_Special4,
    CreatureAnimation_Imp_Idle4 = CreatureAnimation_Special5,
    CreatureAnimation_Imp_Idle5 = CreatureAnimation_Special6,
    CreatureAnimation_Imp_Idle6 = CreatureAnimation_Special7,
    CreatureAnimation_Imp_Dig2 = CreatureAnimation_Special8,
    CreatureAnimation_Dwarf_ClaimWall = 11,
    CreatureAnimation_Dwarf_ClaimLand = 12,
    CreatureAnimation_Dwarf_Drag = 18,
    CreatureAnimation_Vampire_IntoBat = CreatureAnimation_Special1,
    CreatureAnimation_Vampire_EatCorpse1 = CreatureAnimation_Special2,
    CreatureAnimation_Vampire_EatCorpse2 = CreatureAnimation_Special3,
    CreatureAnimation_Mistress_Whip = CreatureAnimation_Special1,
    CreatureAnimation_Skeleton_Attention = CreatureAnimation_Special1,
    CreatureAnimation_DarkAngel_Aggressive = CreatureAnimation_Special1,
    CreatureAnimation_Knight_Happy = CreatureAnimation_Special3,
    CreatureAnimation_Thief_Loot = CreatureAnimation_Special3,
    CreatureAnimation_Rogue_Loot = CreatureAnimation_Special3,
};

// Get creature animation type string representation
// @param creatureAnimation: Identifier
inline const char* ToString(CreatureAnimationID creatureAnimation)
{
    switch (creatureAnimation)
    {
        case CreatureAnimation_Walk: return "walk";
        case CreatureAnimation_Run: return "run";
        case CreatureAnimation_Dragged_Pose: return "dragged_pose";
        case CreatureAnimation_Recoil_Forwards: return "recoil_forwards";
        case CreatureAnimation_Melee: return "melee";
        case CreatureAnimation_Magic: return "magic";
        case CreatureAnimation_Die: return "die";
        case CreatureAnimation_Happy: return "happy";
        case CreatureAnimation_Angry: return "angry";
        case CreatureAnimation_Stunned_Pose: return "stunned_pose";
        case CreatureAnimation_Swing: return "swing";
        case CreatureAnimation_Sleep_Pose: return "sleep_pose";
        case CreatureAnimation_Eat: return "eat";
        case CreatureAnimation_Research: return "research";
        case CreatureAnimation_Torture: return "torture";
        case CreatureAnimation_Drink: return "drink";
        case CreatureAnimation_Idle1: return "idle1";
        case CreatureAnimation_Recoil_Backwards: return "recoil_backwards";
        case CreatureAnimation_Building: return "building";
        case CreatureAnimation_Pray: return "pray";
        case CreatureAnimation_Fallback: return "fallback";
        case CreatureAnimation_Elec: return "elec";
        case CreatureAnimation_Electrocute: return "electrocute";
        case CreatureAnimation_Getup: return "getup";
        case CreatureAnimation_Dance: return "dance";
        case CreatureAnimation_Drunk1: return "drunk1";
        case CreatureAnimation_Entrance: return "entrance";
        case CreatureAnimation_Idle2: return "idle2";
        case CreatureAnimation_Special1: return "special1";
        case CreatureAnimation_Special2: return "special2";
        case CreatureAnimation_Drunk2: return "drunk2";
        case CreatureAnimation_Special3: return "special3";
        case CreatureAnimation_DrunkIdle: return "drunk_idle";
        case CreatureAnimation_Melee2: return "melee2";
        case CreatureAnimation_Special4: return "special4";
        case CreatureAnimation_Special5: return "special5";
        case CreatureAnimation_Special6: return "special6";
        case CreatureAnimation_Special7: return "special7";
        case CreatureAnimation_Special8: return "special8";
        case CreatureAnimation_WalkBack: return "walk_back";
        case CreatureAnimation_Pose_Frame: return "pose_frame";
        case CreatureAnimation_Walk2: return "walk2";
        case CreatureAnimation_Death_Pose: return "death_pose";

        case CreatureAnimation_Null_NotUsed1:
        case CreatureAnimation_Null_NotUsed2:
        case CreatureAnimation_Null_NotUsed3: 
        case CreatureAnimation_Null_NotUsed4:
            return "not_used";
    }
    cxx_assert(false);
    return "";
}