//-----------------------------------------------------------------------------
// File : BloomFirstDownScalePS.hlsl
// Desc : Bloom First DownScale Pass.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
cbuffer CbParam : register(b0)
{
    float2  InvSize;
    float2  Reserved;
};
Texture2D    ColorMap : register(t0);
SamplerState ColorSmp : register(s0);

//-----------------------------------------------------------------------------
//      輝度値を求めます.
//-----------------------------------------------------------------------------
float BT2020_Luminance(float3 color)
{ return dot(float3(0.2627f, 0.678f, 0.0593f), color); }

//-----------------------------------------------------------------------------
//      Anti-Flickeringの重みを求めます.
//-----------------------------------------------------------------------------
float KarisWeight(float3 color)
{ return 1.0f / (BT2020_Luminance(color) + 1.0f); }

//-----------------------------------------------------------------------------
//      ダウンサンプルを行います.
//-----------------------------------------------------------------------------
float3 DownSample(float2 uv)
{
    // [Jimenez 2014] Jorge Jimenez, "Next Generation Post Processing in Call of Duty Advanced Warfare"
    // SIGGRAPH 2014: Advanced in Real-Time Rendering Courses, Slide.164-170.

    // VGPRを無駄に使わないように気を付ける.
    float3 result = float3(0.0f, 0.0f, 0.0f);
    float  weight = 0.0f;

    // 0.5 weights, 0.5f offset.
    float3 c = ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x, -InvSize.y) * 0.5f);
    result += c * 0.5f;
    weight += KarisWeight(c);

    c = ColorMap.Sample(ColorSmp, uv + float2(InvSize.x, -InvSize.y) * 0.5f);
    result += c * 0.5f;
    weight += KarisWeight(c);

    c = ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x, InvSize.y) * 0.5f);
    result += c * 0.5f;
    weight += KarisWeight(c);

    c = ColorMap.Sample(ColorSmp, uv + float2(InvSize.x, InvSize.y) * 0.5f);
    result += c * 0.5f;
    weight += KarisWeight(c);

    // 0.125 weights, 1.0 offset.
    c = ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x, -InvSize.y));
    result += c * 0.125f;
    weight += KarisWeight(c);

    c = ColorMap.Sample(ColorSmp, uv + float2(0.0f, -InvSize.y));
    result += c * 0.25f; // 0.125 * 2.0 = 0.25;
    weight += KarisWeight(c) * 2.0f;

    c = ColorMap.Sample(ColorSmp, uv + float2(InvSize.x, -InvSize.y));
    result += c * 0.125f;
    weight += KarisWeight(c);

    c = ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x, 0.0f));
    result += c * 0.25f; // 0.125 * 2.0 = 0.25;
    weight += KarisWeight(c) * 2.0f;

    c = ColorMap.Sample(ColorSmp, uv);
    result += c * 0.5f; // 0.125 * 4.0 = 0.5;
    weight += KarisWeight(c) * 4.0f;

    c = ColorMap.Sample(ColorSmp, uv + float2(InvSize.x, 0.0f));
    result += c * 0.25f; // 0.125 * 2.0 = 0.25;
    weight += KarisWeight(c) * 2.0f;

    c = ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x, InvSize.y));
    result += c * 0.125f;
    weight += KarisWeight(c);

    c = ColorMap.Sample(ColorSmp, uv + float2(0.0f, InvSize.y));
    result += c * 0.25f; // 0.125 * 2.0 = 0.5;
    weight += KarisWeight(c) * 2.0f;

    c = ColorMap.Sample(ColorSmp, uv + float2(InvSize.x, InvSize.y));
    result += c * 0.125f;
    weight += KarisWeight(c);

    // トーンマップの重みで割る.
    if (weight > 0.0f)
    { result /= weight; }

    return result;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    return float4(DownSample(input.TexCoord), 1.0f);
}
