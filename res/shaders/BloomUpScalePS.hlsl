//-----------------------------------------------------------------------------
// File : BloomUpScalePS.hlsl
// Desc : Bloom UpSampling.
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
Texture2D    ColorMap : register(t0);
SamplerState ColorSmp : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // 3x3 Tent Filter.
    result += ColorMap.Sample(ColorSmp, input.TexCoord, int2(-1, -1));
    result += ColorMap.Sample(ColorSmp, input.TexCoord, int2( 0, -1)) * 2.0f;
    result += ColorMap.Sample(ColorSmp, input.TexCoord, int2( 1, -1));

    result += ColorMap.Sample(ColorSmp, input.TexCoord, int2(-1,  0)) * 2.0f;
    result += ColorMap.Sample(ColorSmp, input.TexCoord, int2( 0,  0)) * 4.0f;
    result += ColorMap.Sample(ColorSmp, input.TexCoord, int2( 1,  0)) * 2.0f;

    result += ColorMap.Sample(ColorSmp, input.TexCoord, int2(-1,  1));
    result += ColorMap.Sample(ColorSmp, input.TexCoord, int2( 0,  1)) * 2.0f;
    result += ColorMap.Sample(ColorSmp, input.TexCoord, int2( 1,  1));

    return result * 0.0625f;
}