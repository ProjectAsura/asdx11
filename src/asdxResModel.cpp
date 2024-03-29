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

    resource.Normals.clear();
    resource.Normals.shrink_to_fit();

    resource.Tangents.clear();
    resource.Tangents.shrink_to_fit();

    resource.Bitangents.clear();
    resource.Bitangents.shrink_to_fit();

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

//-----------------------------------------------------------------------------
//      法線ベクトルを計算します.
//-----------------------------------------------------------------------------
void CalcNormals(ResMesh& resource)
{
    auto vertexCount = resource.Positions.size();
    std::vector<asdx::Vector3> normals;
    normals.resize(vertexCount);

    // 法線データ初期化.
    for(size_t i=0; i<vertexCount; ++i)
    {
        normals[i] = asdx::Vector3(0.0f, 0.0f, 0.0f);
    }

    auto indexCount = resource.Indices.size();
    for(size_t i=0; i<indexCount - 3; i+=3)
    {
        auto i0 = resource.Indices[i + 0];
        auto i1 = resource.Indices[i + 1];
        auto i2 = resource.Indices[i + 2];

        const auto& p0 = resource.Positions[i0];
        const auto& p1 = resource.Positions[i1];
        const auto& p2 = resource.Positions[i2];

        // エッジ.
        auto e0 = p1 - p0;
        auto e1 = p2 - p0;

        // 面法線を算出.
        auto fn = asdx::Vector3::Cross(e0, e1);
        fn = asdx::Vector3::SafeNormalize(fn, fn);

        // 面法線を加算.
        normals[i0] += fn;
        normals[i1] += fn;
        normals[i2] += fn;
    }

    // 加算した法線を正規化し，頂点法線を求める.
    for(size_t i=0; i<vertexCount; ++i)
    {
        normals[i] = asdx::Vector3::SafeNormalize(normals[i], normals[i]);
    }

    const auto SMOOTHING_ANGLE = 59.7f;
    auto cosSmooth = cosf(asdx::ToDegree(SMOOTHING_ANGLE));

    // メモリ確保.
    resource.Normals.resize(vertexCount);

    // スムージング処理.
    for(size_t i=0; i<indexCount - 3; i+=3)
    {
        auto i0 = resource.Indices[i + 0];
        auto i1 = resource.Indices[i + 1];
        auto i2 = resource.Indices[i + 2];

        const auto& p0 = resource.Positions[i0];
        const auto& p1 = resource.Positions[i1];
        const auto& p2 = resource.Positions[i2];

        // エッジ.
        auto e0 = p1 - p0;
        auto e1 = p2 - p0;

        // 面法線を算出.
        auto fn = asdx::Vector3::Cross(e0, e1);
        fn = asdx::Vector3::SafeNormalize(fn, fn);

        // 頂点法線と面法線のなす角度を算出.
        auto c0 = asdx::Vector3::Dot(normals[i0], fn);
        auto c1 = asdx::Vector3::Dot(normals[i1], fn);
        auto c2 = asdx::Vector3::Dot(normals[i2], fn);

        // スムージング処理.
        resource.Normals[i0] = (c0 >= cosSmooth) ? normals[i0] : fn;
        resource.Normals[i1] = (c1 >= cosSmooth) ? normals[i1] : fn;
        resource.Normals[i2] = (c2 >= cosSmooth) ? normals[i2] : fn;
    }

    normals.clear();
}

//-----------------------------------------------------------------------------
//      法線ベクトルを計算します.
//-----------------------------------------------------------------------------
void CalcNormals(ResModel& resource)
{
    for(auto& mesh : resource.Meshes)
    { CalcNormals(mesh); }
}

//-----------------------------------------------------------------------------
//      接線ベクトルを計算します.
//-----------------------------------------------------------------------------
void CalcTangentsRoughly(ResMesh& mesh)
{
    auto vertexCount = mesh.Positions.size();
    mesh.Tangents.resize(vertexCount);
    for(size_t i=0; i<vertexCount; ++i)
    {
        asdx::Vector3 T, B;
        asdx::CalcONB(mesh.Normals[i], T, B);
        mesh.Tangents[i] = T;
    }
}
//-----------------------------------------------------------------------------
//      接線ベクトルを計算します.
//-----------------------------------------------------------------------------
void CalcTangents(ResMesh& resource)
{
    // テクスチャ座標が無い場合は接線ベクトルをきちんと計算できないので，
    // 雑に計算する.
    if (resource.TexCoords[0].empty())
    {
        CalcTangentsRoughly(resource);
        return;
    }

    auto vertexCount = resource.Positions.size();
    resource.Tangents.resize(vertexCount);

    // 接線ベクトルを初期化.
    for(size_t i=0; i<vertexCount; ++i)
    {
        resource.Tangents[i] = asdx::Vector3(0.0f, 0.0f, 0.0f);
    }

    auto indexCount = resource.Indices.size();
    for(size_t i=0; i<indexCount - 3; i+=3)
    {
        auto i0 = resource.Indices[i + 0];
        auto i1 = resource.Indices[i + 1];
        auto i2 = resource.Indices[i + 2];

        auto p0 = resource.Positions[i0];
        auto p1 = resource.Positions[i1];
        auto p2 = resource.Positions[i2];

        auto t0 = resource.TexCoords[0][i0];
        auto t1 = resource.TexCoords[0][i1];
        auto t2 = resource.TexCoords[0][i2];

        auto e1 = p1 - p0;
        auto e2 = p2 - p0;

        float x1 = t1.x - t0.x;
        float x2 = t2.x - t0.x;

        float y1 = t1.y - t0.y;
        float y2 = t2.y - t0.y;

        float r = 1.0f / (x1 * y2 - x2 * y1);

        asdx::Vector3 T = (e1 * y2 - e2 * y1) * r;

        resource.Tangents[i0] += T;
        resource.Tangents[i1] += T;
        resource.Tangents[i2] += T;
    }

    for(size_t i=0; i<vertexCount; ++i)
    {
        // Reject = a - b * Dot(a, b);
        auto a = resource.Tangents[i];
        auto b = resource.Normals[i];
        auto T = a - b * asdx::Vector3::Dot(a, b);
        resource.Tangents[i] = asdx::Vector3::SafeNormalize(T, T);
    }
}

//-----------------------------------------------------------------------------
//      接線ベクトルを計算します.
//-----------------------------------------------------------------------------
void CalcTangents(ResModel& resource)
{
    for(auto& mesh : resource.Meshes)
    { CalcTangents(mesh); }
}

//-----------------------------------------------------------------------------
//      カラーを圧縮します.
//-----------------------------------------------------------------------------
uint32_t EncodeColor(const asdx::Vector4& value)
{
    Unorm8888 packed = {};
    packed.r = uint8_t(value.x * 255.0f);
    packed.g = uint8_t(value.y * 255.0f);
    packed.b = uint8_t(value.z * 255.0f);
    packed.a = uint8_t(value.w * 255.0f);
    return packed.c;
}

//-----------------------------------------------------------------------------
//      圧縮されたカラーを展開します.
//-----------------------------------------------------------------------------
asdx::Vector4 DecodeColor(uint32_t value)
{
    Unorm8888 packed = {};
    packed.c = value;
    return Vector4(
        float(packed.r) / 255.0f,
        float(packed.g) / 255.0f,
        float(packed.b) / 255.0f,
        float(packed.a) / 255.0f);
}

} // namespace asdx
