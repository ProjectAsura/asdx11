//-----------------------------------------------------------------------------
// File : BloomDownScalePS.hlsl
// Desc : Bloom DownScale Pass.
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
//      ダウンサンプルを行います.
//-----------------------------------------------------------------------------
float3 DownSample(float2 uv)
{
    // [Jimenez 2014] Jorge Jimenez, "Next Generation Post Processing in Call of Duty Advanced Warfare"
    // SIGGRAPH 2014: Advanced in Real-Time Rendering Courses, Slide.164-170.

    // VGPRを無駄に使わないように気を付ける.
    float3 result = float3(0.0f, 0.0f, 0.0f);

    // 0.5 weights, 0.5f offset.
    result += ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x, -InvSize.y) * 0.5f) * 0.5f;
    result += ColorMap.Sample(ColorSmp, uv + float2( InvSize.x, -InvSize.y) * 0.5f) * 0.5f;
    result += ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x,  InvSize.y) * 0.5f) * 0.5f;
    result += ColorMap.Sample(ColorSmp, uv + float2( InvSize.x,  InvSize.y) * 0.5f) * 0.5f;

    // 0.125 weights, 1.0 offset.
    result += ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x, -InvSize.y)) * 0.125f;
    result += ColorMap.Sample(ColorSmp, uv + float2(0.0f,       -InvSize.y)) * 0.25f;
    result += ColorMap.Sample(ColorSmp, uv + float2(InvSize.x,  -InvSize.y)) * 0.125f;

    result += ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x, 0.0f)) * 0.25f;
    result += ColorMap.Sample(ColorSmp, uv) * 0.5f;
    result += ColorMap.Sample(ColorSmp, uv + float2( InvSize.x, 0.0f)) * 0.25f;

    result += ColorMap.Sample(ColorSmp, uv + float2(-InvSize.x, InvSize.y)) * 0.125f;
    result += ColorMap.Sample(ColorSmp, uv + float2(0.0f,       InvSize.y)) * 0.25f;
    result += ColorMap.Sample(ColorSmp, uv + float2( InvSize.x, InvSize.y)) * 0.125f;

    return result;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    return float4(DownSample(input.TexCoord), 1.0f);
}
