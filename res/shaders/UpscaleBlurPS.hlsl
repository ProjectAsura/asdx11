//-------------------------------------------------------------------------------------------------
// File : UpdateScalePS.hlsl
// Desc : Pixel Shader For Downscale Buffer with Upscaling.
// Copyright(c) Project Asura. All right resreved.
//-------------------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

//-------------------------------------------------------------------------------------------------
// Textures and Samplers.
//-------------------------------------------------------------------------------------------------
Texture2D       ColorMap : register(t0);
SamplerState    ColorSmp : register(s0);


//-------------------------------------------------------------------------------------------------
//      メインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    float2 invSize;
    ColorMap.GetDimensions(invSize.x, invSize.y);
    invSize.xy = 1.0f / invSize.xy;

    float2 uv = input.TexCoord;
    float4 c0 = ColorMap.SampleLevel(ColorSmp, uv + invSize * float2(-0.5f, -0.5f), 0);
    float4 c1 = ColorMap.SampleLevel(ColorSmp, uv + invSize * float2( 0.5f, -0.5f), 0);
    float4 c2 = ColorMap.SampleLevel(ColorSmp, uv + invSize * float2(-0.5f,  0.5f), 0);
    float4 c3 = ColorMap.SampleLevel(ColorSmp, uv + invSize * float2( 0.5f,  0.5f), 0);

    return (c0 + c1 + c2 + c3) * 0.25f;
}