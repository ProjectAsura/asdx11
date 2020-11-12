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
TextureCube  Texture0 : register(t0);

//-----------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
float4 main(const PSInput input) : SV_TARGET0
{
    float3 dir = float3(0.0f, 0.0f, 0.0f);

    // ��.
    if (input.TexCoord.y > 2.0f)
    {
        dir.x = input.TexCoord.x - 1.0f;
        dir.y = 1.0f;
        dir.z = input.TexCoord.y - 2.0f;

    }
    // ��.
    else if (input.TexCoord.y < 0.0f)
    {
        dir.x = input.TexCoord.x - 1.0f;
        dir.y = -1.0f;
        dir.z = input.TexCoord.y - 2.0f;
    }
    else
    {
        // ��
        if (input.TexCoord.x < 3.0f && input.TexCoord.x <= 4.0f)
        {
            dir.x = (1.0f - input.TexCoord.x);
            dir.y = input.TexCoord.y;
            dir.z = -1.0f;

        }
        // �E
        else if (input.TexCoord.x < 2.0f && input.TexCoord.x <= 3.0f)
        {
            dir.x = 1.0f;
            dir.y = input.TexCoord.y;
            dir.z = input.TexCoord.x;

        }
        // �O.
        else if (input.TexCoord.x < 1.0f && input.TexCoord.x <= 2.0f)
        {
            dir.x = input.TexCoord.x;
            dir.y = input.TexCoord.y;
            dir.z = 1.0f;
        }
        // ��
        else
        {
            dir.x = -1.0f;
            dir.y = input.TexCoord.y;
            dir.z = input.TexCoord.x;
        }
    }

    float4 texColor = Texture0.Sample(Sampler0, dir);
    return texColor * input.Color;
}
