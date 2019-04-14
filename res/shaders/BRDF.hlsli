//-----------------------------------------------------------------------------
// File : BRDF.hlsli
// Desc : BRDF.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef BRDF_HLSLI
#define BRDF_HLSLI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"



//-----------------------------------------------------------------------------
//      ディフューズ反射率を求めます.
//-----------------------------------------------------------------------------
float3 ToKd(float3 baseColor, float metallic)
{ return (1.0f - metallic) * baseColor; }

//-----------------------------------------------------------------------------
//      スペキュラー反射率を求めます.
//-----------------------------------------------------------------------------
float3 ToKs(float3 baseColor, float metallic)
{ return lerp(0.03f, baseColor, metallic); }

//-----------------------------------------------------------------------------
//      非金属向け.
//-----------------------------------------------------------------------------
float3 ToKsDielectics(float3 baseColor, float metallic, float reflectance)
{ return lerp(0.16f * reflectance * reflectance, baseColor, metallic); }

//-----------------------------------------------------------------------------
//      金属向け.
//-----------------------------------------------------------------------------
float3 ToKsConductors(float3 baseColor, float metallic)
{ return baseColor * metallic; }

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
//      Schlickによるフレネル反射の近似値を求める.
//-----------------------------------------------------------------------------
float F_Schlick(in float f0, in float f90, in float u)
{ return f0 + (f90 - f0) * pow(1.0f - u, 5.0f); }

//-----------------------------------------------------------------------------
//      フレネル項を計算します.
//-----------------------------------------------------------------------------
float3 F_Schlick(const float3 f0, float VoH)
{
    float f = pow(1.0f - VoH, 5.0f);
    return f + f0 * (1.0f - f);
}

//-----------------------------------------------------------------------------
//      フレネル項を計算します.
//-----------------------------------------------------------------------------
float F_Schlick(const float f0, float VoH)
{
    float f = pow(1.0f - VoH, 5.0f);
    return f + f0 * (1.0f - f);
}

//-----------------------------------------------------------------------------
//      ディフューズの支配的な方向を求めます.
//-----------------------------------------------------------------------------
float3 GetDiffuseDominantDir(float3 N, float3 V, float NoV, float roughness)
{
    float a = 10.2341f * roughness - 1.51174f;
    float b = -0.511705f * roughness + 0.755868f;
    float lerpFactor = saturate((NoV * a + b) * roughness);
    return lerp(N, V, lerpFactor);
}

//-----------------------------------------------------------------------------
//      スペキュラーの支配的な方向を求めます.
//-----------------------------------------------------------------------------
float3 GetSpecularDomiantDir(float3 N, float3 R, float roughness)
{
    float smoothness = saturate(1.0f - roughness);
    float lerpFactor = smoothness * (sqrt(smoothness) + roughness);
    return lerp(N, R, lerpFactor);
}

//-----------------------------------------------------------------------------
//      スペキュラーAOを計算します.
//-----------------------------------------------------------------------------
float CalcSpecularAO(float NoV, float ao, float roughness)
{ return saturate(pow(max(NoV + ao, 0.0f), exp2(-16.0f * roughness - 1.0f)) - 1.0f + ao); }

//-----------------------------------------------------------------------------
//      水平スペキュラーAOを計算します.
//-----------------------------------------------------------------------------
float CalcHorizonAO(float RoN)
{
    // CalcSpecularAOがパフォーマンス的に困る場合に，簡易な近似として使用する.
    float horizon = min(1.0f + RoN, 1.0f);
    return horizon * horizon;
}

//-----------------------------------------------------------------------------
//      マイクロシャドウを計算します.
//-----------------------------------------------------------------------------
float ApplyMicroShadow(float ao, float NoL, float shadow)
{
    // See, "The Technical Art of Uncharted 4"
    float aperture = 2.0 * ao * ao;
    float microShadow = saturate(NoL + aperture - 1.0);
    return shadow * microShadow;
}

//-----------------------------------------------------------------------------
//      Lambert Diffuseを求めます.
//-----------------------------------------------------------------------------
float LambertDiffuse(float NoL)
{ return NoL / F_PI; }

//-----------------------------------------------------------------------------
//      Disney Diffuseを求めます.
//-----------------------------------------------------------------------------
float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness)
{
    float  energyBias   = lerp(0.0f, 0.5f, roughness);
    float  energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);
    float  fd90 = energyBias + 2.0f * LdotH * LdotH * roughness;
    float  lightScatter = F_Schlick(1.0f, fd90, NdotL).r;
    float  viewScatter  = F_Schlick(1.0f, fd90, NdotV).r;

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

//-------------------------------------------------------------------------------------------------
//      Height Correlated SmithによるG項を求めます.
//-------------------------------------------------------------------------------------------------
float G_SmithGGX(float NdotL, float NdotV, float alphaG)
{
    float alphaG2 = alphaG * alphaG;
    float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
    float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);
    return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

//-----------------------------------------------------------------------------
//      標準モデルのDFG項のフィッティング近似です.
//-----------------------------------------------------------------------------
float2 ApproxDFG(float roughness, float NoV)
{
    // Karis' approximation based on Lazarov's
    const float4 c0 = float4(-1.0, -0.0275, -0.572, 0.022);
    const float4 c1 = float4(1.0, 0.0425, 1.040, -0.040);
    float4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
    return float2(-1.04, 1.04) * a004 + r.zw;
}

//-----------------------------------------------------------------------------
//      AshikhminモデルのDFG項のフィッティング近似です.
//-----------------------------------------------------------------------------
float2 ApproxDFGClothAshikhmin(float roughness, float NoV)
{
    const float4 c0 = float4(0.24, 0.93, 0.01, 0.20);
    const float4 c1 = float4(2.00, -1.30, 0.40, 0.03);

    float s = 1.0f - NoV;
    float e = s - c0.y;
    float g = c0.x * exp2(-(e * e) / (2.0 * c0.z)) + s * c0.w;
    float n = roughness * c1.x + c1.y;
    float r = max(1.0 - n * n, c1.z) * g;

    return float2(r, r * c1.w);
}

//-----------------------------------------------------------------------------
//      CharlieモデルのDFG項のフィッティング近似です.
//-----------------------------------------------------------------------------
float2 ApproxDFGClothCharlie(float roughness, float NoV)
{
    const float3 c0 = float3(0.95f, 1250.0f, 0.0095f);
    const float4 c1 = float4(0.04f, 0.2f, 0.3f, 0.2f);

    float a = 1.0f - NoV;
    float b = 1.0f - roughness;

    float n = pow(max(c1.x + a, 0.0f), 64.0);
    float e = b - c0.x;
    float g = exp2(-(e * e) * c0.y);
    float f = b + c1.y;
    float a2 = a * a;
    float a3 = a2 * a;
    float c = n * g + c1.z * (a + c1.w) * roughness + f * f * a3 * a3 * a2;
    float r = min(c, 18.0);

    return float2(r, r * c0.z);
}

//-----------------------------------------------------------------------------
//      D項を計算します.
//-----------------------------------------------------------------------------
float D_Ashikhmin(float linearRoughness, float NoH)
{
    // Ashkhmin 2007, "Distribution-based BRDFs".
    float a2 = linearRoughness * linearRoughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0f - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 0 in fp16
    float sin4h = sin2h * sin2h;
    float cot2 = -cos2h / (a2 * sin2h);
    return 1.0f / (F_PI * (4.0f * a2 + 1.0f) * sin4h) * (4.0f * exp(cot2) + sin4h);
}

//-----------------------------------------------------------------------------
//      D項を計算します.
//-----------------------------------------------------------------------------
float D_Charlie(float linearRoughness, float NoH)
{
    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF".
    float invAlpha = 1.0f / linearRoughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0f - cos2h, 0.0078125f); // 2^(-14/2), so sin2h^2 0 in fp16
    return (2.0f + invAlpha) * pow(sin2h, invAlpha * 0.5f) / (2.0f * F_PI);
}

//-----------------------------------------------------------------------------
//      V項を計算します.
//-----------------------------------------------------------------------------
float V_Neubelt(float NoV, float NoL)
{
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for THe Order: 1886".
    return saturate(1.0f / (4.0f * (NoL + NoV - NoL * NoV)));
}

//-----------------------------------------------------------------------------
//      布用ディフューズ項を評価します.
//-----------------------------------------------------------------------------
float3 EvaluateClothDiffuse(float3 diffuseColor, float sheen, float VoH)
{ 
#if 0
    return (diffuseColor / F_PI) * (1.0f - F_Schlick(sheen, VoH));
#else
    return (diffuseColor / F_PI);
#endif
}

//-----------------------------------------------------------------------------
//      布用スペキュラー項を評価します.
//-----------------------------------------------------------------------------
float EvaluateClothSpecular
(
    float   sheen,
    float   clothness,
    float   NoH,
    float   NoL,
    float   NoV,
    float   LoH
)
{
    float D = D_Charlie(clothness, NoH);
    float V = V_Neubelt(NoV, NoL);
    float F = F_Schlick(sheen, LoH);
    return (D * V * F) / F_PI * NoL;
}

//-----------------------------------------------------------------------------
//      ヘアのスペキュラー項を求めます.
//-----------------------------------------------------------------------------
float ScheuermannSingleSpecularTerm(float3 T, float3 H, float exponent)
{
    // Thorsten Scheuermann, "Hair Rendering and Shading", ShaderX 3, p.244　参照.
    float ToH   = dot(T, H);
    float sinTH = sqrt(1.0f - ToH * ToH);
    return pow(sinTH, exponent);
}

//-----------------------------------------------------------------------------
//      ヘアのスペキュラー減衰を求めます.
//-----------------------------------------------------------------------------
float ScheuermannSpecularAttenuation(float NoL)
{
    // Thorsten Scheuermann, "Hair Rendering and Shading", ShaderX 3. p.246 参照.
    return saturate(1.75f * NoL + 0.25f);
}

//-----------------------------------------------------------------------------
//      Kajiya-Keyディフューズ項を評価します.
//-----------------------------------------------------------------------------
float EvaluateScheuermannDiffuse(float NoL)
{
    // Thorsten Scheuermann, "Hair Rendering and Shading", ShaderX 3, p.243 参照.
    return saturate(0.75f * NoL + 0.25f);
}

//-----------------------------------------------------------------------------
//      Scheuermannスペキュラー項を評価します.
//-----------------------------------------------------------------------------
float3 EvaluateScheuermannSpecular
(
    float3  T,              // 接線ベクトル.
    float3  N,              // 法線ベクトル.
    float3  H,              // 視線ベクトルとライトベクトルのハーフベクトル.
    float   NoL,            // 法線とライトベクトルの内積.
    float4  specularColor0, // RGB : スペキュラーカラー, A : 反射強度.
    float4  specularColor1, // RGB : スペキュラーカラー, A : 反射強度.
    float   specularShift0, // シフト量.
    float   specularShift1, // シフト量.
    float   shiftValue,     // シフトテクスチャの値.
    float   noise           // ノイズテクスチャの値.
)
{
    float3 T0 = ShiftTangent(T, N, specularShift0 + shiftValue);
    float3 T1 = ShiftTangent(T, N, specularShift1 + shiftValue);

    float3 specular0 = specularColor0.rgb * ScheuermannSingleSpecularTerm(T0, H, specularColor0.a);
    float3 specular1 = specularColor1.rgb * ScheuermannSingleSpecularTerm(T1, H, specularColor1.a);

    specular1 *= noise;

    return (specular0 + specular1) * ScheuermannSpecularAttenuation(NoL);
}

//-----------------------------------------------------------------------------
//      Kajiya-Kay BRDFを評価します.
//-----------------------------------------------------------------------------
float3 EvaluateKajiyaKay
(
    float3  T,              // 接線ベクトル.
    float3  N,              // 法線ベクトル.
    float3  V,              // 視線ベクトル.
    float3  L,              // ライトベクトル.
    float3  baseColor,      // ベースカラー.
    float   noise           // ノイズテクスチャの値.
)
{
    // James T. Kajiya, Timothy L. Kay, "RENDERING FUR WITH THREE DIMENSIONAL TEXTURES",
    // Computer Graphics, Volume 23, Number 3, July 1989,
    // Diffuse  は Equation (14) 参照.
    // Specular は Equation (16) 参照.

    // ワケあって固定値.
    const float Kd              = 0.4f;
    const float Ks0             = 0.14f;
    const float Ks1             = 0.15f;
    const float SpecularPower0  = 80.0f;
    const float SpecularPower1  = 8.0f;

    float cosTL = dot(T, L);
    float sinTL = ToSin(cosTL);

    float diffuse = sinTL;
    float alpha   = radians(noise * 10.0f); // チルト角(5 - 10 度)

    float cosTRL = -cosTL;
    float sinTRL =  sinTL;
    float cosTV  = dot(T, V);
    float sinTV  = ToSin(cosTV);

    // プライマリーカラーを求める.
    float cosTRL0   = cosTRL * cos(2.0f * alpha) - sinTRL * sin(2.0f * alpha);
    float sinTRL0   = ToSin(cosTRL0);
    float specular0 = max(0, cosTRL0 * cosTV + sinTRL0 * sinTV);

    // セカンダリーカラーを求める.
    float cosTRL1   = cosTRL * cos(-3.0f * alpha) - sinTRL * sin(-3.0f * alpha);
    float sinTRL1   = ToSin(cosTRL1);
    float specular1 = max(0, cosTRL1 * cosTV + sinTRL1 * sinTV);

    // BRDFを評価.
    float3 result = Kd * diffuse * baseColor;
    result += Ks0 * pow(specular0, SpecularPower0);
    result += Ks1 * pow(specular1, SpecularPower1) * baseColor;

    return result;
}

//-----------------------------------------------------------------------------
//      V項を計算します.
//-----------------------------------------------------------------------------
float V_Kelemen(float LoH)
{ return 0.25f / max((LoH * LoH), 1e-5f); }

//-----------------------------------------------------------------------------
//      クリアコートのラフネスを求めます.
//-----------------------------------------------------------------------------
float GetClearCoatRoughness(float clearCoatRoughness)
{ return lerp(0.089f, 0.6f, clearCoatRoughness); }

//-----------------------------------------------------------------------------
//      クリアコートのフレネル項を求めます.
//-----------------------------------------------------------------------------
float GetClearCoatFresnel(float LoH, float clearCoat)
{ return F_Schlick(0.04f, 1.0f, LoH) * clearCoat; }

//-----------------------------------------------------------------------------
//      異方性GGXのD項を求めます.
//-----------------------------------------------------------------------------
float D_GGXAnisotropic
(
    float  at,
    float  ab,
    float  NoH,
    float3 H,
    float3 T,
    float3 B
)
{
    //float  at  = max(linearRoughness * (1.0f + anisotropy), 0.001f);
    //float  ab  = max(linearRoughness * (1.0f - anisotropy), 0.001f);
    float  ToH = dot(T, H);
    float  BoH = dot(B, H);
    float  a2  = at * ab;
    float3 v   = float3(ab * ToH, at * BoH, a2 * NoH);
    float  v2  = dot(v, v);
    float  w2  = a2 / v2;

    return a2 * w2 * w2 * (1.0f / F_PI);
}

//-----------------------------------------------------------------------------
//      異方性GGXのV項を求めます.
//-----------------------------------------------------------------------------
float V_SmithGGXHeightCorrelatedAnisotropic
(
    float at,
    float ab,
    float3 V,
    float3 L,
    float3 T,
    float3 B,
    float NoV,
    float NoL
)
{
    //float  at  = max(linearRoughness * (1.0f + anisotropy), 0.001f);
    //float  ab  = max(linearRoughness * (1.0f - anisotropy), 0.001f);
    float ToV = dot(T, V);
    float BoV = dot(B, V);
    float ToL = dot(T, L);
    float BoL = dot(B, L);

    float lambdaV = NoL * length(float3(at * ToV, ab * BoV, NoV));
    float lambdaL = NoV * length(float3(at * ToL, ab * BoL, NoL));
    float v = 0.5f / (lambdaV + lambdaL);
    return saturate(v);
}

//-----------------------------------------------------------------------------
//      計算により眼球用の高さを求めます.
//-----------------------------------------------------------------------------
float ProcedualHeightForEye(float radius, float anteriorChamberDepth)
{
    // Jorge Jimenez, Javier von der Pahlen,
    // "Next-Generation Character Rendering", GDC 2013
    // Eye Rendering セクション参照.
    //const float anteriorChamberDepth = 3.23f; // 3.23[nm] from [Lackner 2005]
    return anteriorChamberDepth * saturate(1.0f - 18.4f * radius * radius);
}

//-----------------------------------------------------------------------------
//      屈折ベクトルを計算します.
//-----------------------------------------------------------------------------
float3 CalcRefraction(float ior, float3 N, float3 V)
{
    // "Real-time Rendering Third Edition", 9.5 Refraction, p.396
    // 式(9.31), 式(9.32)参照.
    float w = ior * dot(N, V);
    float k = sqrt(1.0f + (w - ior) * (w + ior));
    return (w - k) * N - ior * V;
}

//-----------------------------------------------------------------------------
//      視差マッピングによる屈折後のテクスチャ座標を計算します.
//-----------------------------------------------------------------------------
float2 ParallaxRefraction
(
    float2      texcoord,               // テクスチャ座標.
    float       radius,                 // 眼球の半径.
    float       parallaxScale,          // 視差スケール.
    float       anteriorChamberDepth,   // 前房の深さ.
    float3      viewW,                  // ワールド空間の視線ベクトル.
    float4x4    world                   // ワールド行列.
)
{
    // Jorge Jimenez, Javier von der Pahlen,
    // "Next-Generation Character Rendering", GDC 2013
    // Eye Rendering セクション参照.
    float2 viewL = mul(viewW, (float3x2)world);
    float2 offset = ProcedualHeightForEye(radius, anteriorChamberDepth) * viewL;
    offset.y = -offset.y;
    return texcoord - parallaxScale * offset;
}

//-----------------------------------------------------------------------------
//      物理ベースによる屈折後のテクスチャ座標を計算します.
//-----------------------------------------------------------------------------
float2 PhysicallyBasedRefraction
(
    float2      texcoord,           // テクスチャ座標.
    float       radius,             // 眼球の半径. 
    float       mask,               // 網膜から強膜への補間値.
    float       anteriorChamberDepth,
    float       ior,                // 屈折率.
    float3      normalW,            // 法線ベクトル.
    float3      viewW,              // ワールド空間での視線ベクトル.
    float3      frontNormalW,       // 眼球の視線ベクトル
    float4x4    world               // ワールド行列
)
{
    // 参考. Real-Time Rendering Third Edition, Section 9.5 Refractions.
    float w = ior * dot(normalW, viewW);
    float k = sqrt(max(1.0f + (w - ior) * (w + ior), 0.0f));
    float3 refractedW = (w - k) * normalW - ior * viewW;

    // Jorge Jimenez, Javier von der Pahlen,
    // "Next-Generation Character Rendering", GDC 2013
    // Eye Rendering セクション参照.
    float  cosAlpha = dot(frontNormalW, -refractedW);
    float  dist     = ProcedualHeightForEye(radius, anteriorChamberDepth) / cosAlpha;
    float3 offsetW  = dist * refractedW;
    float2 offsetL  = mul(offsetW, (float3x2)world); // ローカルに変換.
    return texcoord + float2(mask, -mask) * offsetL;
}

//-----------------------------------------------------------------------------
//      薄ガラスを評価します.
//-----------------------------------------------------------------------------
void EvaluateThinGlass
(
    in  float   eta,                // 屈折率.
    in  float   NoV,                // 法線と視線ベクトルの内積.
    in  float3  baseColor,          // ベースカラー.
    out float3  transmittance,      // トランスミッタンス.
    out float3  reflectance,        // リフレクタンス.
    out float3  absorptionRatio     // 吸収率.
)
{
    float sinTheta2 = 1.0f - NoV * NoV;

    const float sinRefractedTheta2 = sinTheta2 / (eta * eta);
    const float cosRefractedTheta  = sqrt(1 - sinRefractedTheta2);

    const float q0 = mad(eta, cosRefractedTheta, -NoV);
    const float q1 = mad(eta, cosRefractedTheta, NoV);
    const float q2 = mad(eta, NoV, -cosRefractedTheta);
    const float q3 = mad(eta, NoV, cosRefractedTheta);

    const float r0 = q0 / q1;
    const float r1 = q2 / q3;

    // 入射面におけるフレネルリフレクタンス.
    const float R0 = 0.5 * saturate(r0 * r0 + r1 * r1);
    // 入射面におけるフレネルトランスミッタンス.
    const float T0 = 1 - R0;

    const float3 R = float3(R0, R0, R0);
    const float3 T = float3(T0, T0, T0);
    const float3 C = float3(cosRefractedTheta, cosRefractedTheta, cosRefractedTheta);

    // 吸収を考慮するための係数.
    const float3 K = pow(max(baseColor, 0.001), 1 / C);
    const float3 RK = R * K;

    transmittance   = saturate(T * T * K / (1 - RK * RK));
    reflectance     = saturate(RK  * transmittance + R);
    absorptionRatio = saturate(-(1 + RK) * transmittance + T);

    // reflectanceを出力カラーに乗算.
    // transmittanceは合成する背景色を下記式を用いて変更.
    // backGround = lerp(1.0, transmittance, alpha);
    // これを使ってアルファブレンディングする.
}

float F_Schlick_ClearCoat(float value)
{
    return 0.04f + 0.96 * pow(1.0f - value, 5.0f);
}

float3 EvaluateDirectLightClearCoat
(
    float3  N,
    float3  L,
    float3  V,
    float3  Kd,
    float3  Ks,
    float   roughness,
    float   clearCoatStrength,
    float   clearCoatRoughness
)
{
    float3 H = normalize(V + L);
    float LoH = saturate(dot(L, H));
    float NoL = saturate(dot(N, L));
    float NoH = saturate(dot(N, H));
    float NoV = saturate(dot(N, V));
    float HoV = saturate(dot(H, V));
    float a2  = max(roughness * roughness, 0.01f);
    float f90 = saturate(50.0f * dot(Ks, 0.33f));

    float3 standard_diffuse = (Kd / F_PI);
    float  standard_D = D_GGX(NoH, a2);
    float  standard_G = G_SmithGGX(NoL, NoV, a2);
    float3 standard_F = F_Schlick(Ks, f90, LoH);
    float3 standard_specular = Ks * (standard_D * standard_G * standard_F) / F_PI;

    float3 standard = (standard_diffuse + standard_specular);
    float clearCoat_F = max(F_Schlick_ClearCoat(NoV), F_Schlick_ClearCoat(NoL));

    float ac2 = GetClearCoatRoughness(clearCoatRoughness * clearCoatRoughness);
    float3 flakly_base = standard * (1.0f - clearCoatStrength * clearCoat_F);

    float  coating_D = D_GGX(NoH, ac2);
    float  coating_G = G_SmithGGX(NoL, NoV, ac2);
    float  coating_F = F_Schlick_ClearCoat(HoV) * clearCoatStrength;
    float3 coating   = (coating_D * coating_G * coating_F) / F_PI;

    return (flakly_base + coating) * NoL;
}



#endif//BRDF_HLSLI
