//-----------------------------------------------------------------------------
// File : PrimitivePS.hlsl
// Desc : Pixel Shader For Primitive with Shadowing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position     : SV_POSITION;
    float4 Color        : COLOR;
    float4 ShadowCoord  : SHADOW_COORD;
};

///////////////////////////////////////////////////////////////////////////////
// CbShadow Constant Buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbShadow : register(b2)
{
    float4x4    ShadowMatrix;
    float4      ShadowColor;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D               ShadowMap : register(t0);
SamplerComparisonState  ShadowSmp : register(s0);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float  z  = saturate(input.ShadowCoord.z / input.ShadowCoord.w);
    float2 uv = input.ShadowCoord.xy / input.ShadowCoord.w;
    float  shadow = ShadowMap.SampleCmpLevelZero(ShadowSmp, uv, z);

    float3 color = lerp(ShadowColor.rgb, input.Color.rgb, shadow);

    return float4(color, 1.0f);
}