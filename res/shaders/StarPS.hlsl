//-----------------------------------------------------------------------------
// File : StarPS.hlsl
// Desc : Pixel Shader For Star Effect(Light Streak)
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
cbuffer CbParam : register(b0)
{
    float4 Offsets[4];  // xy: offset, z: weight, w: reserved.
    float4 Colors [4];  // 乗算カラー.
};
Texture2D    ColorMap : register(t0);
SamplerState ColorSmp : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

    [unroll]
    for(int i=0; i<4; ++i)
    { result += ColorMap.SampleLevel(ColorSmp, input.TexCoord + Offsets[i].xy, 0.0f) * Offsets[i].z * Colors[i]; }

    return result;
}
