#ifndef MATH_HLSLI
#define MATH_HLSLI

static const float gPI = 3.141592653;
static const float g2PI = 6.28318530718;
static const float gEps = 1e-8;
static const float gM1PI = 0.318309886183790671538;

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

float3 RandomUnitVector(inout uint seed)
{
    return normalize(RandomInUnitSphere(seed));
}

float3 RandomCosineDirection(inout uint seed)
{
    float r1 = RandomFloat01(seed);
    float r2 = RandomFloat01(seed);
    float z = sqrt(1.0f-r2);

    float phi = 2*gPI*r1;
    float sr2 = sqrt(r2);
    float x = cos(phi) * sr2;
    float y = sin(phi) * sr2;

    return float3(x, y, z);
}

float3 RandomToDisk(inout uint seed) {
    float Theta = RandomFloat01(seed);
    return float3(cos(g2PI * Theta), sin(g2PI * Theta), 0.0f);
}

float3 RandomToSphere(float radius, float distanceSquared, inout uint seed) {
    float r1 = RandomFloat01(seed);
    float r2 = RandomFloat01(seed);
    float z = 1 + r2*(sqrt(1-radius*radius/distanceSquared) - 1);

    float phi = 2*gPI*r1;
    float x = cos(phi)*sqrt(1-z*z);
    float y = sin(phi)*sqrt(1-z*z);

    return float3(x, y, z);
}

float3x3 GetONB(float3 w)
{
    float3 axisZ, axisY, axisX;
    axisZ = w;
    float3 l = abs(w.x) > 0.9 ? float3(0.0f, 1.0f, 0.0f) : float3(1.0f, 0.0f, 0.0f);
    axisY = normalize(cross(w, l));
    axisX = normalize(cross(w, axisY));
    return float3x3 (axisX, axisY, axisZ);
}

bool NearZero(float3 e) {
    return (abs(e[0]) < gEps) && (abs(e[1]) < gEps) && (abs(e[2]) < gEps);
}

bool InsideRay(float3 rayDirection, float3 surfaceNormal) {
    return dot(rayDirection, surfaceNormal) > 0.0f;
}

float3 Refract(float3 indir, float3 normal, float ir) {
    float cos_theta = min(dot(-indir, normal), 1.0f);
    float3 r_out_perp =  ir * (indir + cos_theta*normal);
    float3 r_out_parallel = -sqrt(abs(1.0f - dot(r_out_perp, r_out_perp))) * normal;
    return r_out_perp + r_out_parallel;
}

float2 VectorToLatLong(float3 dir)
{
    float3 p = normalize(dir);
    float u = (1.f + atan2(p.x, -p.z) * gM1PI) * 0.5f;
    float v = acos(p.y) * gM1PI;
    return float2(u, v);
}

bool HitSphere(float3 center, float radius, float3 origin, float3 direction) {
    float3 oc = origin - center;
    float a = dot(direction, direction);
    float b = 2.0 * dot(oc, direction);
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4*a*c;
    return discriminant > 0;
}

float PowerHeuristic(uint numf, float fPdf, uint numg, float gPdf) {
    float f = numf * fPdf;
    float g = numg * gPdf;

    return (f * f) / (f * f + g * g);
}
#endif