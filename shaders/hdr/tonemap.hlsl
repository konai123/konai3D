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
    //(0, 0), (1, 0), (1, 1)
    vout.Coord = float2((vid << 1) & 2, vid & 2);
    //NDC
    vout.Position = float4(vout.Coord.x * 2.0f - 1.0f, -vout.Coord.y * 2.0f + 1.0f, 0.0f, 1.0f);
    return vout;
}

float4 PS(VertexOut vIn) : SV_Target {
    float4 c = gShaderInput.SampleLevel(gSamPointClamp, vIn.Coord, 0.0f);
    return float4(c.xyz, 1.0f);
}

#endif