//-----------------------------------------------------------------------------
// File : ImguiVS.hlsl
// Desc : Vertex Shader For Imgui.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    float2 Position : POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// PSInput structure
///////////////////////////////////////////////////////////////////////////////
struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// CbTransform constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbTransform : register(b0)
{
    float4x4 ProjectionMatrix;
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
PSInput main(const VSInput input)
{
    PSInput output;
    output.Position = mul(ProjectionMatrix, float4(input.Position, 0.0f, 1.0f));
    output.Color    = input.Color;
    output.TexCoord = input.TexCoord;
    return output;
}
