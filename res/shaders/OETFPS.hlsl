//-----------------------------------------------------------------------------
// File : OETFPS.hlsl
// Desc : Opt-Electronic Transfer Functions.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////
// CbOETF constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbOETF : register(b0)
{
    int   OETFType          : packoffset(c0);
    float BaseLuminance     : packoffset(c0.y);
    float TargetLuminance   : packoffset(c0.z);
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D    ColorMap : register(t0);
SamplerState ColorSmp : register(s0);


float3 EOTF_SRGB(float3 color)
{ return (color < 0.04045) ? color / 12.92 : pow(abs(color + 0.055) / 1.055, 2.4); }

// LinearからTV側への伝送.
float3 OETF_Gamma(float3 color, float gamma)
{ return pow(color, 1.0f / gamma); }

// LinearからTV側 SRGBへの伝送.
float3 OETF_SRGB(float3 color)
{ return (color <= 0.0031308) ? 12.92f * color : (1.0f + 0.055f) * pow(abs(color), 1.0f / 2.4f) - 0.055f; }

// LinearからTV側 AdobeRGBへの伝送.
float3 OETF_AdobeRGB(float3 color)
{ return pow(color, 256.0f / 563.0f); }

// LinearからTV側 SMPTE 170M(あるいはBT.2020 10bit)の伝送
float3 OETF_170M(float3 color)
{ return (color <= 0.018f) ? 4.500 * color : (1.0f + 0.099) * pow(abs(color), 0.45f) - 0.099f; }

// LinearからTV側 DCI-P3への伝送
float3 OETF_DCI_P3(float3 color)
{ return pow(color, 1.0f / 2.6f); }

// LinearからTV側 SMPTE ST2084(PQカーブ)への伝送
float3 OETF_PQ(float3 color)
{
    float m1 = 0.1593017578125f;
    float m2 = 78.84375f;
    float c1 = 0.8359375f;
    float c2 = 18.8515625f;
    float c3 = 18.6875f;
    float3 Lm = pow(abs(color), m1);
    return pow((c1 + c2 * Lm) / (1 + c3 * Lm), m2);
}

// LinearからTV側Hybrid Log Gammaへの伝送
float3 OETF_HLG(float3 color)
{
    float a = 0.17883277f;
    float b = 0.28466892f;
    float c = 0.55991073f;
    float r = 0.5f;
    
    return (color <= 1.0f) ? r * sqrt(color) : a * log(color - b) + c;
}

// Linear からTV側 Display-P3への伝送.
float3 OETF_Display_P3(float3 color)
{ return (color <= 0.0030186f) ? 12.92 * color : (1.055f * pow(abs(color), 1.0f/2.4f)) - 0.055f; }


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(VSOutput input) : SV_TARGET0
{
    float3 color = ColorMap.SampleLevel(ColorSmp, input.TexCoord, 0).rgb;

    switch(OETFType)
    {
    case 1:
        color = OETF_SRGB(color);
        break;

    case 2:
        color = OETF_170M(color);
        break;

    case 3:
        color = OETF_PQ(color * TargetLuminance / 10000.0f);
        break;

    case 4:
        color = OETF_HLG(color);
        break;

    default:
        break;
    }

    return float4(color, 1.0f);
}
