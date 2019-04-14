//-----------------------------------------------------------------------------
// File : SkySpherePS.hlsl
// Desc : Pixel Shader For SkySphere.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4      Position     : SV_POSITION;
    float2      TexCoord     : TEXCOORD;
    float3      Tangent      : TANGENT;
    float3      Binormal     : BINORMAL;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D    SphereMap : register(t0);
SamplerState SphereSmp : register(s0);

///////////////////////////////////////////////////////////////////////////////
// CbSkySphereFlow
///////////////////////////////////////////////////////////////////////////////
cbuffer CbSkySphereFlow : register(b1)
{
    float3 WindDirection : packoffset(c0);
    float  Offset0       : packoffset(c1);
    float  Offset1       : packoffset(c1.y);
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    float2 flowDir = float2(
        dot(WindDirection, input.Tangent),
        dot(WindDirection, input.Binormal));

    float phase0 = Offset0;
    float phase1 = Offset1;

    float4 color0 = SphereMap.Sample(SphereSmp, input.TexCoord + flowDir * phase0);
    float4 color1 = SphereMap.Sample(SphereSmp, input.TexCoord + flowDir * phase1);

    float  factor = abs(0.5f - Offset0) / 0.5f;
    float3 output = lerp(color0.rgb, color1.rgb, factor);

    return float4(output, 0.0f);
}