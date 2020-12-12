//-----------------------------------------------------------------------------
// File : asdxColorMatrix.h
// Desc : Color Matrix.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxMath.h>


namespace asdx {

//-----------------------------------------------------------------------------
//      明度を調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline Matrix CreateBrightnessMatrix(float brightness)
{ return Matrix::CreateScale(brightness); }

//-----------------------------------------------------------------------------
//      彩度を調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline Matrix CreateSaturationMatrix(float r, float g, float b)
{
    // https://docs.microsoft.com/ja-jp/windows/win32/direct2d/saturation
    return Matrix(
        0.213f + 0.787f * r,
        0.213f - 0.213f * r,
        0.213f - 0.213f * r,
        0.0f,

        0.715f - 0.715f * g,
        0.715f + 0.285f * g,
        0.715f - 0.715f * g,
        0.0f,

        0.072f - 0.072f * b,
        0.072f - 0.072f * b,
        0.072f + 0.928f * b,
        0.0f,

        0.0f, 0.0f, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------
//      彩度を調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline Matrix CreateSaturationMatrix(float saturation)
{ return CreateSaturationMatrix(saturation, saturation, saturation); }

//-----------------------------------------------------------------------------
//      コントラストを調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline Matrix CreateContrastMatrix(float contrast)
{
    const auto t = (1.0f - contrast) * 0.5f;
    return Matrix(
        contrast, 0.0f, 0.0f, 0.0f,
        0.0f, contrast, 0.0f, 0.0f,
        0.0f, 0.0f, contrast, 0.0f,
        t, t, t, 1.0f);
}

//-----------------------------------------------------------------------------
//      色相を調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline Matrix CreateHueMatrix(float hue)
{
    // https://docs.microsoft.com/ja-jp/windows/win32/direct2d/hue-rotate
    auto rad = ToRadian(hue);
    auto u   = cosf(rad);
    auto w   = sinf(rad);

    return Matrix(
        0.213f + 0.787f * u - 0.213f * w,
        0.213f - 0.213f * u + 0.143f * w,
        0.213f - 0.213f * u - 0.787f * w,
        0.0f,

        0.715f - 0.715f * u - 0.715f * w,
        0.715f + 0.285f * u + 0.140f * w,
        0.715f - 0.715f * u - 0.283f * w,
        0.0f,

        0.072f - 0.072f * u + 0.928f * w,
        0.072f - 0.072f * u - 0.283f * w,
        0.072f + 0.928f * u + 0.072f * w,
        0.0f,

        0.0f,
        0.0f,
        0.0f,
        1.0f);
}

//-----------------------------------------------------------------------------
//      セピアカラーを調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline Matrix CreateSepiaMatrix(float tone)
{
    const Vector3 W(0.298912f, 0.586611f, 0.114478f);
    const Vector3 Sepia(0.941f, 0.784f, 0.569f);

    return Matrix(
        tone * W.x * Sepia.x + (1.0f - tone),
        tone * W.x * Sepia.y,
        tone * W.x * Sepia.z,
        0.0f,

        tone * W.y * Sepia.x,
        tone * W.y * Sepia.y + (1.0f - tone),
        tone * W.y * Sepia.z,
        0.0f,

        tone * W.z * Sepia.x,
        tone * W.z * Sepia.y,
        tone * W.z * Sepia.z + (1.0f - tone),
        0.0f,

        0.0f, 0.0f, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------
//      グレースケールカラーを調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline Matrix CreateGrayScaleMatrix(float tone)
{
    const Vector3 GrayScale(0.22015f, 0.706655f, 0.071330f);
    return Matrix(
        tone * GrayScale.x + (1.0f - tone),
        tone * GrayScale.x,
        tone * GrayScale.x,
        0.0f,

        tone * GrayScale.y,
        tone * GrayScale.y + (1.0f - tone),
        tone * GrayScale.y,
        0.0f,

        tone * GrayScale.z,
        tone * GrayScale.z,
        tone * GrayScale.z + (1.0f - tone),
        0.0f,

        0.0f, 0.0f, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------
//      ネガポジ反転のカラー行列を生成します.
//-----------------------------------------------------------------------------
inline Matrix CreateNegaposiMatrix()
{
    return Matrix(
        -1.0f,  0.0f,  0.0f, 0.0f,
         0.0f, -1.0f,  0.0f, 0.0f,
         0.0f,  0.0f, -1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f);
}

} // namespace asdx