//-----------------------------------------------------------------------------
// File : BloomCompositePS.hlsl
// Desc : Bloom Composite.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D       ColorMap0 : register(t0); // 元データ.
Texture2D       ColorMap1 : register(t1);
Texture2D       ColorMap2 : register(t2);
Texture2D       ColorMap3 : register(t3);
Texture2D       ColorMap4 : register(t4);
Texture2D       ColorMap5 : register(t5);
SamplerState    ColorSmp  : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 result = 0;
    result += ColorMap0.SampleLevel(ColorSmp, input.TexCoord, 0.0f);
    result += ColorMap1.SampleLevel(ColorSmp, input.TexCoord, 0.0f);
    result += ColorMap2.SampleLevel(ColorSmp, input.TexCoord, 0.0f);
    result += ColorMap3.SampleLevel(ColorSmp, input.TexCoord, 0.0f);
    result += ColorMap4.SampleLevel(ColorSmp, input.TexCoord, 0.0f);
    result += ColorMap5.SampleLevel(ColorSmp, input.TexCoord, 0.0f);
    result.w = 1.0f;
    return result;
}
