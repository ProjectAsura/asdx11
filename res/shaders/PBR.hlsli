//-----------------------------------------------------------------------------
// File : PBR.hlsli
// Desc : Physically Based Rendering.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef PBR_HLSLI
#define PBR_HLSLI

//-----------------------------------------------------------------------------
// Constant Values
//-----------------------------------------------------------------------------
static const float F_PI = 3.1415926535897932384626433832795f;
static const float F_1DIVPI = 0.31830988618379067153776752674503f;


///////////////////////////////////////////////////////////////////////////////
// LightBuffer 
///////////////////////////////////////////////////////////////////////////////
cbuffer LightBuffer : register( b0 )
{
    float3 CameraPosition       : packoffset( c0 );     // カメラ位置です.
    float  IBLIntensity         : packoffset( c0.w );   // IBLの強さです.
    float3 DirectLightDirection : packoffset( c1 );     // ライトの照射方向です.
    float  DirectLightIntensity : packoffset( c1.w );   // 直接光の強さです.
};

//-----------------------------------------------------------------------------
//  Textures and Samplers.
//-----------------------------------------------------------------------------
TextureCube     DiffuseLDMap  : register(t0);     // Irrdiance Environment Map.
SamplerState    DiffuseLDSmp  : register(s0);
TextureCube     SpecularLDMap : register(t1);     // Radiance Environment Map.
SamplerState    SpecularLDSmp : register(s1);
Texture2D       DFGMap        : register(t2);     // Ambient BRDF Map.
SamplerState    DFGSmp        : register(s2);


//-----------------------------------------------------------------------------
//      Hammersleyサンプリング.
//-----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N) 
{
    // Shader Model 5以上が必要.
    float ri = reversebits(i) * 2.3283064365386963e-10f;
    return float2(float(i) / float(N), ri);
}

//-----------------------------------------------------------------------------
//      90度入射におけるフレネル反射率を求めます.
//-----------------------------------------------------------------------------
float CalcF90(in float3 f0)
{ return saturate(50.0f * dot(f0, 0.33f)); }

//-----------------------------------------------------------------------------
//      Schlickによるフレネル反射の近似値を求める.
//-----------------------------------------------------------------------------
float3 F_Schlick(in float3 f0, in float f90, in float u)
{ return f0 + (f90 - f0) * pow(1.0f - u, 5.0f); }

//-----------------------------------------------------------------------------
//      Disney Diffuseを求めます.
//-----------------------------------------------------------------------------
float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness)
{
    float  energyBias   = lerp(0.0f, 0.5f, roughness);
    float  energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);
    float  fd90         = energyBias + 2.0f * LdotH * LdotH * roughness;
    float3 f0           = float3(1.0f, 1.0f, 1.0f);
    float  lightScatter = F_Schlick(f0, fd90, NdotL).r;
    float  viewScatter  = F_Schlick(f0, fd90, NdotV).r;

    return lightScatter * viewScatter * energyFactor;
}

//-----------------------------------------------------------------------------
//      GGXのD項を求めます.
//-----------------------------------------------------------------------------
float D_GGX(float NdotH, float m)
{
    float m2 = m * m;
    float f = (NdotH * m2 - NdotH) * NdotH + 1;
    return m2 / (f * f);
}

//-----------------------------------------------------------------------------
//      Height Correlated SmithによるG項を求めます.
//-----------------------------------------------------------------------------
float G_SmithGGX(float NdotL, float NdotV, float alphaG)
{
    float alphaG2 = alphaG * alphaG;
    float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
    float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);
    return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

//-----------------------------------------------------------------------------
//      ディフューズを評価します.
//-----------------------------------------------------------------------------
float3 EvaluateDiffuse(float3 N, float3 Kd)
{
    return Kd * IrradianceEnvMap.SampleLevel(IrradianceEnvSmp, N, 0).rgb * IBLIntensity;
}

//-----------------------------------------------------------------------------
//      スペキュラーを評価します.
//-----------------------------------------------------------------------------
float3 EvaluateSpecular(float3 V, float3 N, float roughness)
{
    // ミップマップレベル数を取得.
    float2 size;
    float mipLevels;
    RadianceEnvMap.GetDimensions( 0, size.x, size.y, mipLevels );

    // 参照ミップマップレベルをroughnessから算出.
    float mipIndex = roughness * (mipLevels - 1.0f);
    float3 R = reflect(V, N);

    return RadianceEnvMap.SampleLevel(RadianceEnvSmp, R, mipIndex).rgb * IBLIntensity;
}

//-----------------------------------------------------------------------------
//      IBLを評価します.
//-----------------------------------------------------------------------------
float3 EvaluateIBL(float3 N, float3 V, float3 Kd, float3 Ks, float roughness)
{
    // Diffuse  : Lambert.
    // Specular : GGX
    float3 irradiance = EvaluateDiffuse(N, Kd);
    float3 radiance   = EvaluateSpecular(V, N, roughness);
    float  shininess  = 1.0f - roughness;

    float3 DFG = AmbientBRDFMap.SampleLevel(AmbientBRDFSmp, float2(dot(V, N), shininess), 0).rgb;

    return (irradiance + radiance * (Ks * DFG.r + DFG.g));
}

//-----------------------------------------------------------------------------
//      直接光を評価します.
//-----------------------------------------------------------------------------
float3 EvaluateDirectLight(float3 N, float3 V, float3 L, float3 Kd, float3 Ks, float roughness)
{
    float  NdotV = abs(dot(N, V)) + 1e-5;
    float3 H     = normalize(V + L);
    float  LdotH = saturate(dot(L, H));
    float  NdotH = saturate(dot(N, H));
    float  NdotL = saturate(dot(N, L));
    float  VdotH = saturate(dot(V, H));
    float  a2    = max(roughness * roughness, 0.01);
    float  f90   = saturate(50.0f * dot(Ks, 0.33f));

    float3 diffuse = Kd / F_PI;
    float  D = D_GGX(NdotH, a2);
    float  G = G_SmithGGX(NdotL, NdotV, a2);
    float3 F = F_Schlick(Ks, f90, LdotH);
    float3 specular = (D * G * F) / F_PI;

    return (diffuse + specular) * NdotL * DirectLightIntensity;
}

//-----------------------------------------------------------------------------
//      cos項に応じた重点サンプリングを行います.
//-----------------------------------------------------------------------------
void ImportanceSampleCos
(
    in float2  u,       // 乱数.
    in float3  N,       // 法線ベクトル.
    out float3 L,       // ライトベクトル.
    out float  NdotL,   // 法線とライトベクトルの内積
    out float  pdf      // 確率密度関数.
)
{
    float3 upward = (abs(N.z) < 0.999) ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 T = normalize(cross(upward, N));
    float3 B = cross(N, T);

    float u1 = u.x;
    float u2 = u.y;

    float r = sqrt(u1);
    float phi = u2 * F_PI * 2.0f;

    L = float3(r * cos(phi), r * sin(phi), sqrt(max(0.0f, 1.0f - u1)));
    L = normalize(T * L.x + B * L.y + N * L.z);

    NdotL = dot(L, N);
    pdf = NdotL * F_1DIVPI;
}

//-----------------------------------------------------------------------------
//      GGX D項に応じた重点サンプリングを行います.
//-----------------------------------------------------------------------------
float3 ImportanceSampleGGX
(
    in float2 u,    // 乱数.
    in float  a,    // a = roughness * roughness.
    in float3 N     // 法線ベクトル.
)
{
    float phi = 2.0f * F_PI * u.x;
    float cosTheta = sqrt((1.0f - u.y) / (1.0f + (a * a - 1.0f) * u.y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    float3 L;
    L.x = sinTheta * cos(phi);
    L.y = sinTheta * sin(phi);
    L.z = cosTheta;

    float3 upward = (abs(N.z) < 0.999) ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 T = normalize(cross(upward, N));
    float3 B = cross(N, T);

    return T * L.x + B * L.y + N * L.z;
}


#endif//PBR_HLSLI
