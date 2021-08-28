#ifndef VERTEX_HLSLI
#define VERTEX_HLSLI

struct Vertex
{
    float3 Position;
    float3 Normal;
    float3 TangentU;
    float2 TexCoord;
};

Vertex GetVertexAttributes(Vertex v0, Vertex v1, Vertex v2, float3 barycentrics)
{
    float3 position = v0.Position * barycentrics.x + v1.Position * barycentrics.y + v2.Position * barycentrics.z;
    float3 normal = v0.Normal * barycentrics.x + v1.Normal * barycentrics.y + v2.Normal * barycentrics.z;
    float3 tangentU = v0.TangentU * barycentrics.x + v1.TangentU * barycentrics.y + v2.TangentU * barycentrics.z;
    float2 texuv = v0.TexCoord * barycentrics.x + v1.TexCoord * barycentrics.y + v2.TexCoord * barycentrics.z;;
    Vertex v;
    v.Position = position;
    v.Normal = normal;
    v.TangentU = tangentU;
    v.TexCoord = texuv;
    return v;
}

#endif