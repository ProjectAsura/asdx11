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

///////////////////////////////////////////////////////////////////////////////
// CbFont constant buffer
///////////////////////////////////////////////////////////////////////////////
cbuffer CbFont : register(b0)
{
    float4 OutlineColor;    // �����J���[.
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
    float4 result;
    float alpha = FontTexture.Sample(LinearClamp, input.TexCoord);
    if (alpha == 0.0f)
    {
        // �אڃs�N�Z�����`�F�b�N���C����菈�����s��.
        float check = 0.0f;
        check += FontTexture.Sample(LinearClamp, input.TexCoord, int2(-1, 1));
        check += FontTexture.Sample(LinearClamp, input.TexCoord, int2( 0, 1));
        check += FontTexture.Sample(LinearClamp, input.TexCoord, int2( 1, 1));

        check += FontTexture.Sample(LinearClamp, input.TexCoord, int2(-1, 0));
        check += FontTexture.Sample(LinearClamp, input.TexCoord, int2( 1, 0));

        check += FontTexture.Sample(LinearClamp, input.TexCoord, int2(-1, -1));
        check += FontTexture.Sample(LinearClamp, input.TexCoord, int2( 0, -1));
        check += FontTexture.Sample(LinearClamp, input.TexCoord, int2( 1, -1));

        result = (check > 0.0f) ? OutlineColor : float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        result = lerp(OutlineColor, input.Color, alpha);
    }

    return result;
}
