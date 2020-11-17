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

static const float F_PI = 3.1415926535f;

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
SamplerState Sampler0 : register(s0);
TextureCube  Texture0 : register(t0);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const PSInput input) : SV_TARGET0
{
    // [-1, 1]に変更.
    float2 uv = input.TexCoord * float2(2.0f, -2.0f) - float2(1.0f, -1.0f);

    float theta = uv.x * F_PI;
    float phi   = uv.y * F_PI * 0.5f;

    float3 dir;
    dir.x = cos(phi) * cos(theta);
    dir.y = sin(phi);
    dir.z = cos(phi) * sin(theta);

    float4 color = Texture0.Sample(Sampler0, dir);
    return color * input.Color;
}
