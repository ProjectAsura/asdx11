//-----------------------------------------------------------------------------
// File : IBLBakeDFG.hlsli
// Desc : Pixel Shader For Baking Irradiance Environment Map.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "PBR.hlsli"

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const int SampleCount = 32;


///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////
// CbBake buffer
///////////////////////////////////////////////////////////////////////////////
cbuffer CbBake : register(b0)
{
    int     FaceIndex : packoffset(c0);
    float   Roughness : packoffset(c0.y);
    int     Width     : packoffset(c0.z);
    float   MipCount  : packoffset(c0.w);
};

//-----------------------------------------------------------------------------
//      キューブマップのフェッチ方向を求めます.
//-----------------------------------------------------------------------------
float3 CalcDirection(float2 uv, int face_index)
{
     float3 dir = 0;
     float2 pos = uv * 2.0f - 1.0f;
     
     switch(face_index)
     {
         case 0 : { dir = float3( 1.0f,  -pos.y,  -pos.x); } break; // PX
         case 1 : { dir = float3(-1.0f,  -pos.y,   pos.x); } break; // NX
         case 2 : { dir = float3( pos.x,   1.0f,   pos.y); } break; // PY
         case 3 : { dir = float3( pos.x,  -1.0f,  -pos.y); } break; // NY
         case 4 : { dir = float3( pos.x, -pos.y,    1.0f); } break; // PZ
         case 5 : { dir = float3(-pos.x, -pos.y,   -1.0f); } break; // NZ
     };

     return normalize(dir);
};

//-----------------------------------------------------------------------------
//      DFG項の積分計算を行います.
//-----------------------------------------------------------------------------
float4 IntegrateDFGOnly
(
    in float3 V,
    in float3 N,
    in float  roughness
)
{
    float   NdotV       = saturate(dot(N, V));
    float4  acc         = 0.0f;
    float   accWeight   = 0.0f;
    float   a           = roughness * roughness;

    [loop]
    for(int i=0; i<SampleCount; ++i)
    {
        float2 u        = Hammersley(i, SampleCount);
        float3 L        = 0;
        float  NdotH    = 0;
        float  LdotH    = 0;

        L = ImportanceSampleGGX(u, a, N);
        float NdotL = saturate(dot(N, L));
        float G     = G_SmithGGX(NdotL, NdotV, a);

        if (NdotL > 0 && G > 0 && NdotH > 0)
        {
            float GVis = G * LdotH / (NdotH * NdotV);
            float Fc = pow(1 - LdotH, 5.0f);
            acc.x += (1 - Fc) * GVis;
            acc.y += Fc * GVis;
        }

        u = frac(u + 0.5f);

        float pdf;
        ImportanceSampleCos(u, N, L, NdotL, pdf);
        if (NdotL > 0)
        {
            float LdotH = saturate(dot(L, normalize(V + L)));
            float NdotV = saturate(dot(N, V));
            acc.z += DisneyDiffuse(NdotV, NdotL, LdotH, roughness);
        }

        accWeight += 1.0f;
    }

    return acc * (1.0f / accWeight);
}

//-----------------------------------------------------------------------------
//      メイン関数です.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    float3 dir = CalcDirection(input.TexCoord, FaceIndex);
    return IntegrateDFGOnly(dir, dir, Roughness);
}