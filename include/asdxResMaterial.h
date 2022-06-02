//-----------------------------------------------------------------------------
// File : asdxResMaterial.h
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Inlcudes
//-----------------------------------------------------------------------------
#include <string>
#include <map>
#include <vector>
#include <asdxMath.h>


namespace asdx {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const char* TAG_DIFFUSE      = u8"Diffuse";
static const char* TAG_SPECULAR     = u8"Specular";
static const char* TAG_BASE_COLOR   = u8"BaseColor";
static const char* TAG_NORMAL       = u8"Normal";
static const char* TAG_ORM          = u8"ORM";
static const char* TAG_OCCLUSION    = u8"Occlusion";
static const char* TAG_ROUGHNESS    = u8"Roughness";
static const char* TAG_METALNESS    = u8"Metalness";
static const char* TAG_EMISSIVE     = u8"Emissive";


///////////////////////////////////////////////////////////////////////////////
// PROPERTY_TYPE enum
///////////////////////////////////////////////////////////////////////////////
enum PROPERTY_TYPE
{
    PROPERTY_TYPE_BOOL,
    PROPERTY_TYPE_INT,
    PROPERTY_TYPE_UINT,
    PROPERTY_TYPE_FLOAT,
    PROPERTY_TYPE_FLOAT2,
    PROPERTY_TYPE_FLOAT3,
    PROPERTY_TYPE_FLOAT4,
};

///////////////////////////////////////////////////////////////////////////////
// ResValue structure
///////////////////////////////////////////////////////////////////////////////
union ResValue
{
    bool            Bool;
    int32_t         Int;
    uint32_t        Uint;
    float           Float;
    asdx::Vector2   Float2;
    asdx::Vector3   Float3;
    asdx::Vector4   Float4;
};

///////////////////////////////////////////////////////////////////////////////
// ResProperty structure
///////////////////////////////////////////////////////////////////////////////
struct ResProperty
{
    PROPERTY_TYPE   Type;
    ResValue        Value;
};

///////////////////////////////////////////////////////////////////////////////
// ResMaterial structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterial
{
    std::string                             Name;
    std::map<std::string, ResProperty>      Props;
    std::map<std::string, std::string>      Textures;
};

///////////////////////////////////////////////////////////////////////////////
// ResMaterialSet structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterialSet
{
    std::vector<ResMaterial>    Materials;
};

} // namespace asdx
