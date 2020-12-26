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
    float4  Position : SV_POSITION; //!< 位置座標です.
    float4  Color    : COLOR0;      //!< 頂点カラーです.
    float2  TexCoord : TEXCOORD0;   //!< テクスチャ座標です.
};

///////////////////////////////////////////////////////////////////////////////
// CbFont constant buffer
///////////////////////////////////////////////////////////////////////////////
cbuffer CbFont : register(b0)
{
    float4 OutlineColor;    // 縁取りカラー.
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D<float>    FontTexture : register(t0);
SamplerState        LinearClamp : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 result;
    float alpha = FontTexture.Sample(LinearClamp, input.TexCoord);
    if (alpha == 0.0f)
    {
        // 隣接ピクセルをチェックし，縁取り処理を行う.
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
