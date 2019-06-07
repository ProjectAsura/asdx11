//-----------------------------------------------------------------------------
// File : PrimitiveVS_Shadowing.hlsl
// Desc : Vertex Shader For Primitive with Shadow.
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
    float4 Position     : SV_POSITION;
    float4 Color        : COLOR;
    float4 ShadowCoord  : SHADOW_COORD;
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

///////////////////////////////////////////////////////////////////////////////
// CbShadow Constant Buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbShadow : register(b2)
{
    float4x4    ShadowMatrix;
    float4      ShadowColor;
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

    float4 shadowPos = mul( ShadowMatrix, worldPos );

    output.Position     = projPos;
    output.Color        = Color;
    output.ShadowCoord  = shadowPos;

    return output;
}