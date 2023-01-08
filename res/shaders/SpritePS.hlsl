//-----------------------------------------------------------------------------
// File : SpritePS.hlsl
// Desc : Pixel Shader For Sprite System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4  Position : SV_POSITION;
    float2  TexCoord : TEXCOORD0;
    float4  Color    : COLOR0;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D       ColorMap : register(t0);
SamplerState    ColorSmp : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    return ColorMap.Sample(ColorSmp, input.TexCoord) * input.Color;
}
