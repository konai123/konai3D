#ifndef MATH_HLSLI
#define MATH_HLSLI

#define row_major

static const float gPI = 3.141592653;

uint wang_hash(inout uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

#define MAX_UINT 4294967295
float RandomFloat01(inout uint seed)
{
    return wang_hash(seed) / float(MAX_UINT + 1);
}

float RandomFloat(float min, float max, inout uint seed)
{
    return min + (max - min)* RandomFloat01(seed);
}

#endif