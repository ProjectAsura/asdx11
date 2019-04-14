//-----------------------------------------------------------------------------
// File : BlurPS.hlsl
// Desc : Common Blur Shader.
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

///////////////////////////////////////////////////////////////////////////////
// CbBlur Constant Buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbBlur : register(b0)
{
    float4 Offsets[15];
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D       ColorMap : register(t0);
SamplerState    ColorSmp : register(s0);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 result = 0;

    [unroll]
    for(int i=0; i<15; ++i)
    { result += Offsets[i].z * ColorMap.SampleLevel(ColorSmp, input.TexCoord + Offsets[i].xy, 0); }

    result.w = 1.0f;
    return result;
}