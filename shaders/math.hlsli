#ifndef MATH_HLSLI
#define MATH_HLSLI

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

float3 RandomVector01(inout uint seed)
{
    return float3(RandomFloat01(seed), RandomFloat01(seed), RandomFloat01(seed));
}

float3 RandomVector(float min, float max, inout uint seed)
{
    return float3(RandomFloat(min, max, seed), RandomFloat(min, max, seed), RandomFloat(min, max, seed));
}

float3 RandomInUnitSphere(inout uint seed)
{
    while (true) {
        float3 p = RandomVector(-1.0f, 1.0f, seed);
        if (length(p) >= 1.0f) continue;
        return p;
    }
}

#endif