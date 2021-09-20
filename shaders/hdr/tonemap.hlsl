#ifndef TONEMAP_HLSL
#define TONEMAP_HLSL

Texture2D<float4> gShaderInput : register(t0);
SamplerState gSamPointClamp : register(s0);

struct VertexOut {
    float2 Coord : COORD;
    float4 Position : SV_Position;
};

VertexOut VS(uint vid : SV_VertexID) {
    VertexOut vout;
    vout.Coord = float2((vid << 1) & 2, vid & 2);
    vout.Position = float4(vout.Coord.x * 2.0f - 1.0f, -vout.Coord.y * 2.0f + 1.0f, 0.0f, 1.0f);
    return vout;
}

float4 PS(VertexOut vIn) : SV_Target {
    float3 c = gShaderInput.SampleLevel(gSamPointClamp, vIn.Coord, 0.0f).xyz;
    float3 x = max(0.0f, c - 0.004);
    c = (x * (6.2f * x + 0.5f)) / (x*(6.2f * x + 1.7f) + 0.06f);

    return float4(c, 1.0f);
}

#endif