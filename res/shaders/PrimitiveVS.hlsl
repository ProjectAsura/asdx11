//-----------------------------------------------------------------------------
// File : PrimitiveVS.hlsl
// Desc : Vertex Shader For Primitive.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    float3 Position : POSITION;
};

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

///////////////////////////////////////////////////////////////////////////////
// CbScene Constant Buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbScene : register(b0)
{
    float4x4    View       : packoffset(c0);
    float4x4    Proj       : packoffset(c4);
    float4x4    InvView    : packoffset(c8);
    float4x4    InvProj    : packoffset(c12);
    float4      CameraPos  : packoffset(c16);
};

///////////////////////////////////////////////////////////////////////////////
// CbMesh Constant Buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbMesh : register(b1)
{
    float4x4    World;
    float4      Color;
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main(const VSInput input )
{
    VSOutput output = (VSOutput)0;

    float4 localPos = float4(input.Position, 1.0f);
    float4 worldPos = mul( World, localPos );
    float4 viewPos  = mul( View,  worldPos );
    float4 projPos  = mul( Proj,  viewPos );

    output.Position = projPos;
    output.Color    = Color;

    return output;
}