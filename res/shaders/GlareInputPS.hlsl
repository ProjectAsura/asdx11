//-----------------------------------------------------------------------------
// File : GlareInputPS.hlsl
// Desc : Generate Glare Input.
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
// CbGlareInput Constant Buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbGlareInput : register(b0)
{
    float Threshold : packoffset(c0);
    float Intensity : packoffset(c0.y);
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D       ColorMap : register(t0);
SamplerState    ColorSmp : register(s0);

//-----------------------------------------------------------------------------
//      BT.601における輝度値を求めます.
//-----------------------------------------------------------------------------
float BT601_Luminance(float3 value)
{
    const float3 c = float3(0.299f, 0.587f, 0.114f);
    return dot(value, c);
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 color = ColorMap.SampleLevel(ColorSmp, input.TexCoord, 0);
    float  luma  = BT601_Luminance(color.xyz * Intensity);
    float  scale = max((luma - Threshold), 0);
    return color * scale;
}
