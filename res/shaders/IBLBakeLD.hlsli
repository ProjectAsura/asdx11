//-----------------------------------------------------------------------------
// File : IBLBakeLD.hlsli
// Desc : Pixel Shader For Baking Radiance Environment Map.
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
// Textures and Samplers.
//-----------------------------------------------------------------------------
TextureCube     IBLCube : register(t0);
SamplerState    IBLSmp  : register(s0);


//-----------------------------------------------------------------------------
//      キューブマップのフェッチ方向を求めます.
//-----------------------------------------------------------------------------
float3 CalcDirection(float2 uv, uint face_index)
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
//      LD項の積分計算を行います.
//-----------------------------------------------------------------------------
float3 IntegrateLDOnly
(
    in float3 V,
    in float3 N,
    in float  roughness
)
{
    float3 acc       = 0;
    float  accWeight = 0;
    float  a         = roughness * roughness;

    [loop]
    for(int i=0; i<SampleCount; ++i)
    {
        float2 eta = Hammersley(i, SampleCount);
        float3 H = ImportanceSampleGGX(eta, a, N);
        float3 L = 2 * dot(V, H) * H - V;

        float NdotH  = saturate(dot(N, H));
        float NdotL  = saturate(dot(N, L));
        float LdotH  = saturate(dot(L, H));
        float pdf    = D_GGX(NdotH, a) * (NdotH / (4.0f * F_PI * LdotH));
        float omegaS = 1.0f / (SampleCount * pdf);
        float omegaP = 4.0f * F_PI / (6.0f * Width * Width);

        float mipLevel = clamp(0.5f * log2(omegaS / omegaP), 0.0f, MipCount);
        float4 Li = IBLCube.SampleLevel(IBLSmp, L, mipLevel);

        acc += Li.rgb * NdotL;
        accWeight += NdotL;
    }

    return acc / accWeight;
}

//-----------------------------------------------------------------------------
//      メイン関数です.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    float3 dir = CalcDirection(input.TexCoord, FaceIndex);
    float3 color = IntegrateLDOnly(dir, dir, Roughness);
    return float4(color, 1.0f);
}