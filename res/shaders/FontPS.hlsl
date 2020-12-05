//-----------------------------------------------------------------------------
// File : FontPS.hlsl
// Desc : Pixel Shader For Font.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4  Position : SV_POSITION; //!< �ʒu���W�ł�.
    float4  Color    : COLOR0;      //!< ���_�J���[�ł�.
    float2  TexCoord : TEXCOORD0;   //!< �e�N�X�`�����W�ł�.
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D<float>    FontTexture : register(t0);
SamplerState        LinearClamp : register(s0);

//-----------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 color = FontTexture.Sample(LinearClamp, input.TexCoord).rrrr;
    return color * input.Color;
}
