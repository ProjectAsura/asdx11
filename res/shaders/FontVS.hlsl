//-----------------------------------------------------------------------------
// File : FontVS.hlsl
// Desc : Vertex Shader For Font.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    float3 Position : POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// CbFont constant buffer
///////////////////////////////////////////////////////////////////////////////
cbuffer CbFont : register( b0 )
{
    float4x4 Transform;   //!< 変換行列です.
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main( VSInput input )
{
    VSOutput output = (VSOutput)0;

    // 頂点座標を変換.
    float4 localPos = float4( input.Position, 1.0f );

    // 出力値設定.
    output.Position = mul( Transform, localPos );
    output.Color    = input.Color;
    output.TexCoord = input.TexCoord;

    // ピクセルシェーダに出力.
    return output;
}

