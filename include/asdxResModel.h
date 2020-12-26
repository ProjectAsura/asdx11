﻿//-----------------------------------------------------------------------------
// File : asdxResModel.h
// Desc : Model Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <vector>
#include <asdxMath.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ResBoneIndex structure
///////////////////////////////////////////////////////////////////////////////
struct ResBoneIndex
{
    uint16_t     Index0;    //!< ボーン番号0.
    uint16_t     Index1;    //!< ボーン番号1.
    uint16_t     Index2;    //!< ボーン番号2.
    uint16_t     Index3;    //!< ボーン番号3.

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    ResBoneIndex() = default;

    //-------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //-------------------------------------------------------------------------
    ResBoneIndex(uint16_t i0, uint16_t i1, uint16_t i2, uint16_t i3)
   : Index0(i0), Index1(i1), Index2(i2), Index3(i3)
    { /* DO_NOTHING */ }
};

///////////////////////////////////////////////////////////////////////////////
// ResMesh structure
///////////////////////////////////////////////////////////////////////////////
struct ResMesh
{
    std::string                         MeshName;
    std::string                         MaterialName;
    std::vector<asdx::Vector3>          Positions;          // R32G32B32A32_FLOAT
    std::vector<uint32_t>               TangentSpaces;      // R10B10G10A2_UINTで圧縮済み.
    std::vector<uint32_t>               Colors;             // R8G8B8A8_UNORMで圧縮済み.
    std::vector<uint32_t>               TexCoords[4];       // R16R16_FLOATで圧縮済み.
    std::vector<ResBoneIndex>           BoneIndices;        // R16G16B16A16_UINT.
    std::vector<asdx::Vector4>          BoneWeights;        // R32G32B32A32_FLOAT
    std::vector<uint32_t>               Indices;            // VertexIndices
};


///////////////////////////////////////////////////////////////////////////////
// ResModel structure
///////////////////////////////////////////////////////////////////////////////
struct ResModel
{
    std::vector<ResMesh>        Meshes;
};

//-----------------------------------------------------------------------------
//      メッシュの破棄処理を行います.
//-----------------------------------------------------------------------------
void Dispose(ResMesh& resource);

//-----------------------------------------------------------------------------
//      モデルの破棄処理を行います.
//-----------------------------------------------------------------------------
void Dispose(ResModel& resource);

//-----------------------------------------------------------------------------
//      八面体ラップ処理を行います.
//-----------------------------------------------------------------------------
Vector2 OctWrap(const Vector2& value);

//-----------------------------------------------------------------------------
//      法線ベクトルをパッキングします.
//-----------------------------------------------------------------------------
Vector2 PackNormal(const Vector3& value);

//-----------------------------------------------------------------------------
//      法線ベクトルをアンパッキングします.
//-----------------------------------------------------------------------------
Vector3 UnpackNormal(const Vector2& value);

//-----------------------------------------------------------------------------
//      接線空間を圧縮します.
//-----------------------------------------------------------------------------
uint32_t EncodeTBN(
    const Vector3& normal,
    const Vector3& tangent,
    uint8_t binormalHandedeness);

//-----------------------------------------------------------------------------
//      圧縮された接線空間を展開します.
//-----------------------------------------------------------------------------
void DecodeTBN(
    uint32_t encoded,
    Vector3& tangent,
    Vector3& bitangent,
    Vector3& normal);

//-----------------------------------------------------------------------------
//      カラーを圧縮します.
//-----------------------------------------------------------------------------
uint32_t EncodeColor(const asdx::Vector4& value);

//-----------------------------------------------------------------------------
//      圧縮されたカラーを展開します.
//-----------------------------------------------------------------------------
asdx::Vector4 DecodeColor(uint32_t value);

} // namespace asdx