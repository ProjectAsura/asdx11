//-------------------------------------------------------------------------------------------------
// File : BackgroundPS.hlsl
// Desc : Pixel Shader For Environment Map.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput sturucture
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// CbBackground constant buffer.
///////////////////////////////////////////////////////////////////////////////////////////////////
cbuffer CbBackground : register(b0)
{
    float3 CameraAxisX : packoffset(c0);
    float3 CameraAxisY : packoffset(c1);
    float3 CameraAxisZ : packoffset(c2);
};

//-------------------------------------------------------------------------------------------------
// Textures and Samplers.
//-------------------------------------------------------------------------------------------------
TextureCube  EnvironmentMap : register(t0);
SamplerState EnvironmentSmp : register(s0);


float4 main(const VSOutput input) : SV_TARGET
{
    float3 rayDir = normalize(CameraAxisX * input.TexCoord.x + CameraAxisY * (1.0 - input.TexCoord.y) + CameraAxisZ);
    float4 color  = EnvironmentMap.SampleLevel(EnvironmentSmp, rayDir, 0.0f);
    return float4(color.rgb, 0.0f);
}