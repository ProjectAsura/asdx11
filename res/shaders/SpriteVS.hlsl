//-----------------------------------------------------------------------------
// File : SpriteVS.hlsl
// Desc : Vertex Shader for Sprite System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
cbuffer CbSprite : register(b0) {
    float4x4 Transform;
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main(const VSInput input)
{
    VSOutput output = (VSOutput)0;

    float4 localPos = float4(input.Position, 1.0f);
    float4 projPos = mul(Transform, localPos);

    output.Position = projPos;
    output.TexCoord = input.TexCoord;
    output.Color    = input.Color;

    return output;
}
