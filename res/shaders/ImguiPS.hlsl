//-----------------------------------------------------------------------------
// File : ImguiPS.hlsl
// Desc : Pixel Shader For Imgui.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// PSInput structure
///////////////////////////////////////////////////////////////////////////////
struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
SamplerState Sampler0 : register(s0);
Texture2D    Texture0 : register(t0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const PSInput input) : SV_TARGET0
{
    float4 texColor = Texture0.Sample(Sampler0, input.TexCoord);
    return texColor * input.Color;
}
