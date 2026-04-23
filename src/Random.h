#pragma once

//////////////////////////////////////////////////////////////////////////

namespace Random
{
    
    // init rng seed
    void SetLocalThreadSeed(unsigned int seed);

    // unsigned ints
    unsigned int GenerateUint();
    unsigned int GenerateUint(unsigned int minValue, unsigned int maxValue);

    // ints
    int GenerateInt();
    int GenerateInt(int minValue, int maxValue);

    // floats
    float GenerateFloat01();
    float GenerateFloat();
    float GenerateFloat(float minValue, float maxValue);

    // helpers

    inline float GenerateAngleDegrees()
    {
        const float randomAngleDegrees = GenerateFloat01() * 360.0f;
        return randomAngleDegrees;
    }

    inline cxx::angle_t GenerateAngle()
    {
        return cxx::angle_t::from_degrees(GenerateAngleDegrees());
    }

} // namespace Random

//////////////////////////////////////////////////////////////////////////
