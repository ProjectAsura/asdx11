//-----------------------------------------------------------------------------
// File : FontPS.hlsl
// Desc : Pixel Shader For Font.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4  Position : SV_POSITION; //!< 位置座標です.
    float4  Color    : COLOR0;      //!< 頂点カラーです.
    float2  TexCoord : TEXCOORD0;   //!< テクスチャ座標です.
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D<float>    FontTexture : register(t0);
SamplerState        LinearClamp : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 color = FontTexture.Sample(LinearClamp, input.TexCoord).rrrr;
    return color * input.Color;
}
