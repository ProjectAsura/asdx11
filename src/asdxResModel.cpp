﻿//-----------------------------------------------------------------------------
// File : asdxResModel.cpp
// Desc : Model Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxResModel.h>
#include <asdxLogger.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// TangentSpace
///////////////////////////////////////////////////////////////////////////////
union TangentSpace
{
    struct {
        uint32_t    NormalX             : 10;
        uint32_t    NormalY             : 10;
        uint32_t    CosAngle            : 8;
        uint32_t    CompIndex           : 2;
        uint32_t    TangentHandedness   : 1;
        uint32_t    BinormalHandedness  : 1;
    };
    uint32_t u;
};
static_assert(sizeof(TangentSpace) == sizeof(uint32_t), "TangentSpace Invalid Data Size");

///////////////////////////////////////////////////////////////////////////////
// TexCoord
///////////////////////////////////////////////////////////////////////////////
union TexCoord
{
    struct 
    {
        uint16_t x;
        uint16_t y;
    };
    uint32_t u;
};

///////////////////////////////////////////////////////////////////////////////
// Unorm88
///////////////////////////////////////////////////////////////////////////////
union Unorm8888
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint32_t c;
};

//-----------------------------------------------------------------------------
//      最大成分を取得します.
//-----------------------------------------------------------------------------
inline float Max3(const asdx::Vector3& value)
{ return asdx::Max(value.x, asdx::Max(value.y, value.z)); }

} // namespace


namespace asdx {

//-----------------------------------------------------------------------------
//      メッシュの破棄処理を行います.
//-----------------------------------------------------------------------------
void Dispose(ResMesh& resource)
{
    resource.MeshName.clear();
    resource.MaterialName.clear();

    resource.Positions.clear();
    resource.Positions.shrink_to_fit();

    resource.TangentSpaces.clear();
    resource.TangentSpaces.shrink_to_fit();

    resource.Colors.clear();
    resource.Colors.shrink_to_fit();

    for(auto i=0; i<4; ++i)
    {
        resource.TexCoords[i].clear();
        resource.TexCoords[i].shrink_to_fit();
    }

    resource.BoneIndices.clear();
    resource.BoneIndices.shrink_to_fit();

    resource.BoneWeights.clear();
    resource.BoneWeights.shrink_to_fit();

    resource.Indices.clear();
    resource.Indices.shrink_to_fit();
}


//-----------------------------------------------------------------------------
//      モデルの破棄処理を行います.
//-----------------------------------------------------------------------------
void Dispose(ResModel& resource)
{
    for(size_t i=0; i<resource.Meshes.size(); ++i)
    { Dispose(resource.Meshes[i]); }

    resource.Meshes.clear();
    resource.Meshes.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      八面体ラップ処理を行います.
//-----------------------------------------------------------------------------
Vector2 OctWrap(const Vector2& value)
{
    Vector2 result;
    result.x = (1.0f - abs(value.y)) * (value.x >= 0.0f ? 1.0f : -1.0f);
    result.y = (1.0f - abs(value.x)) * (value.y >= 0.0f ? 1.0f : -1.0f);
    return result;
}

//-----------------------------------------------------------------------------
//      法線ベクトルをパッキングします.
//-----------------------------------------------------------------------------
Vector2 PackNormal(const Vector3& value)
{
    auto n = value / (abs(value.x) + abs(value.y) + abs(value.z));
    Vector2 result(n.x, n.y);
    result = (n.z >= 0.0f) ? result : OctWrap(result);
    result = result * 0.5f + Vector2(0.5f, 0.5f);
    return result;
}

//-----------------------------------------------------------------------------
//      法線ベクトルをアンパッキングします.
//-----------------------------------------------------------------------------
Vector3 UnpackNormal(const Vector2& value)
{
    auto encoded = value * 2.0f - Vector2(1.0f, 1.0f);
    auto n = Vector3(encoded.x, encoded.y, 1.0f - abs(encoded.x) - abs(encoded.y));
    auto t = Saturate(-n.z);
    n.x += (n.x >= 0.0f) ? -t : t;
    n.y += (n.y >= 0.0f) ? -t : t;
    return Vector3::Normalize(n);
}

//-----------------------------------------------------------------------------
//      接線空間を圧縮します.
//-----------------------------------------------------------------------------
uint32_t EncodeTBN(const Vector3& normal, const Vector3& tangent, uint8_t binormalHandedness)
{
    auto packedNormal = PackNormal(normal);

    TangentSpace packed;
    packed.NormalX = uint32_t(packedNormal.x * 1023.0);
    packed.NormalY = uint32_t(packedNormal.y * 1023.0);

    auto tangentAbs = Vector3::Abs(tangent);
    auto maxComp = Max3(tangentAbs);

    Vector3 refVector;
    uint32_t compIndex = 0;
    if (maxComp == tangentAbs.x)
    {
        refVector = Vector3(1.0f, 0.0f, 0.0f);
        compIndex = 0;
    }
    else if (maxComp == tangentAbs.y)
    {
        refVector = Vector3(0.0f, 1.0f, 0.0f);
        compIndex = 1;
    }
    else if (maxComp == tangentAbs.z)
    {
        refVector = Vector3(0.0f, 0.0f, 1.0f);
        compIndex = 2;
    }

    auto orthoA = Vector3::Normalize(Vector3::Cross(normal, refVector));
    auto orthoB = Vector3::Cross(normal, orthoA);
    uint8_t cosAngle = uint8_t((Vector3::Dot(tangent, orthoA) * 0.5f + 0.5f) * 255.0f);
    uint8_t tangentHandedness = (Vector3::Dot(tangent, orthoB) > 0.0001f) ? 1 : 0;

    packed.CompIndex            = compIndex;
    packed.CosAngle             = cosAngle;
    packed.TangentHandedness    = tangentHandedness;
    packed.BinormalHandedness   = binormalHandedness;

    return packed.u;
}

//-----------------------------------------------------------------------------
//      圧縮された接線空間を展開します.
//-----------------------------------------------------------------------------
void DecodeTBN(uint32_t encoded, Vector3& tangent, Vector3& bitangent, Vector3& normal)
{
    TangentSpace packed;
    packed.u = encoded;

    normal = UnpackNormal(Vector2(packed.NormalX / 1023.0f, packed.NormalY / 1023.0f));

    Vector3 refVector;
    uint8_t compIndex = (packed.CompIndex);
    if (compIndex == 0)
    { refVector = Vector3(1.0f, 0.0f, 0.0f); }
    else if (compIndex == 1)
    { refVector = Vector3(0.0f, 1.0f, 0.0f); }
    else if (compIndex == 2)
    { refVector = Vector3(0.0f, 0.0f, 1.0f); }

    float cosAngle = (packed.CosAngle / 255.0f) * 2.0f - 1.0f;
    float sinAngle = sqrt(Saturate(1.0f - cosAngle * cosAngle));
    sinAngle = (packed.TangentHandedness == 0) ? -sinAngle : sinAngle;

    auto orthoA = Vector3::Normalize(Vector3::Cross(normal, refVector));
    auto orhotB = Vector3::Cross(normal, orthoA);
    tangent = Vector3::Normalize((cosAngle * orthoA) + (sinAngle * orhotB));

    bitangent = Vector3::Cross(normal, tangent);
    bitangent = (packed.BinormalHandedness == 0) ? bitangent : -bitangent;
}

} // namespace asdx
