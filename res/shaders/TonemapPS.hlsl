//-----------------------------------------------------------------------------
// File : TonemapPS.hlsl
// Desc : Pixel Shader For Tonemapping.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const int TONEMAP_NONE       = 0;
static const int TONEMAP_GT         = 1;
static const int TONEMAP_INUI       = 2;
static const int TONEMAP_ACES_SDR   = 3;
static const int TONEMAP_ACES_HDR   = 4;


///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////
// CbTonemap buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbTonemap : register(b0)
{
    float MaxLuminance;         //!< �ő�P�x�l.
    float BaseLuminance;        //!< ��P�x�l.
    int   TonemapType;          //!< �g�[���}�b�v�^�C�v.
    float GT_Contrast;          //!< [GT] �R���g���X�g.
    float GT_MidStart;          //!< [GT] ���`��Ԃ̊J�n.
    float GT_MidLength;         //!< [GT] ���`��Ԃ̒���.
    float GT_Toe;               //!< [GT] ���̒��܂�.
    float GT_ToeOffset;         //!< [GT] �g�[�I�t�Z�b�g.
    float Inui_Sensitivity;     //!< [����] ���x���q.
    float Inui_PeakDensity;     //!< [����] �ő�Z�x.
    float Inui_Gradient;        //!< [����] �K�����q.
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D       ColorMap : register(t0);
SamplerState    ColorSmp : register(s0);

//-----------------------------------------------------------------------------
//      ACESFilmicSDR
//-----------------------------------------------------------------------------
float ACESFilmicSDR( float x )
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

//-----------------------------------------------------------------------------
//      ACES Filmic REC.2020 1K
//-----------------------------------------------------------------------------
float ACESFilmicHDR(float x)
{
    const float a = 15.8f;
    const float b = 2.12f;
    const float c = 1.2f;
    const float d = 5.92f;
    const float e = 1.9f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

//-----------------------------------------------------------------------------
//      GT�g�[���}�b�v��K�p���܂�.
//-----------------------------------------------------------------------------
float GtTonemap(float x)
{
    const float k = MaxLuminance / BaseLuminance;

    // ����p�����[�^.
    float P = k;
    float a = GT_Contrast;
    float m = GT_MidStart;
    float l = GT_MidLength;
    float c = GT_Toe;
    float b = GT_ToeOffset;

    float l0 = ((P - m) * l) / a;
    float L0 = m - (m / a);
    float L1 = m + ((1.0f - m) / a);

    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    float w0 = 1.0 - smoothstep(0.0f, m, x);
    float w2 = step(m + l0, x);
    float w1 = 1.0f - w0 - w2;

    float T = m * pow(max(x / m, 0.0f), c) + b;
    float S = P - (P - S1) * exp(CP * (x - S0));
    float L = m + a * (x - m);

    return T * w0 + L * w1 + S * w2;
}

//-------------------------------------------------------------------------
//      ����g�[���}�b�v��K�p���܂�.
//-------------------------------------------------------------------------
float InuiTonemap(float x)
{
    // Masao Inui and Syuji Kashiwa ,
    // "The Approximation of Photographic Characteristic Curve",
    // https://www.jstage.jst.go.jp/article/photogrst1964/39/3/39_3_123/_pdf/-char/ja, 1975.
    // Equation (3).
    return Inui_PeakDensity / (1.0 + pow(max(Inui_Sensitivity * x, 0.0f), -Inui_Gradient));
}

//--------------------------------------------------------------------------
//      �P�x�l�����߂܂�.
//--------------------------------------------------------------------------
float LuminanceBT709(float3 color)
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

//-----------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    float3 color = ColorMap.SampleLevel(ColorSmp, input.TexCoord, 0).rgb;
    float luma = LuminanceBT709(color);

    float tone = 1.0f;
    if (TonemapType == TONEMAP_NONE)
    {
        return float4(color, 1.0f);
    }
    else if (TonemapType == TONEMAP_GT)
    {
        tone = GtTonemap(luma);
    }
    else if (TonemapType == TONEMAP_INUI)
    {
        tone = InuiTonemap(luma);
    }
    else if (TonemapType == TONEMAP_ACES_SDR)
    {
        tone = ACESFilmicSDR(luma);
    }
    else if (TonemapType == TONEMAP_ACES_HDR)
    {
        tone = ACESFilmicHDR(luma);
    }

    return float4(color * (tone / luma), 1.0f);
}
