#ifndef SAMPLER_HLSLI
#define SAMPLER_HLSLI

SamplerState gSamPointWrap        : register(s0);
SamplerState gSamPointClamp       : register(s1);
SamplerState gSamLinearWrap       : register(s2);
SamplerState gSamLinearClamp      : register(s3);
SamplerState gSamAnisotropicWrap  : register(s4);
SamplerState gSamAnisotropicClamp : register(s5);

#endif