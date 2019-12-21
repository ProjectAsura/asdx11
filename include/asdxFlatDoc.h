//-----------------------------------------------------------------------------
// File : asdxFlatDoc.h
// Desc : Flat Document.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>
#include <map>
#include <asdxMath.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// FlatDoc class
///////////////////////////////////////////////////////////////////////////////
class FlatDoc
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    FlatDoc();
    ~FlatDoc();

    bool Load(const char* path);
    bool Save(const char* path);

    int             GetInt      (const char* tag, int   defVal = 0) const;
    bool            GetBool     (const char* tag, bool  defVal = false) const;
    float           GetFloat    (const char* tag, float defVal = 0.0f) const;
    asdx::Vector2   GetVec2     (const char* tag, asdx::Vector2 defVal = asdx::Vector2(0.0f, 0.0f)) const;
    asdx::Vector3   GetVec3     (const char* tag, asdx::Vector3 defVal = asdx::Vector3(0.0f, 0.0f, 0.0f)) const;
    asdx::Vector4   GetVec4     (const char* tag, asdx::Vector4 defVal = asdx::Vector4(0.0f, 0.0f, 0.0f, 0.0f)) const;
    asdx::Matrix    GetMatrix   (const char* tag, asdx::Matrix  defVal = asdx::Matrix::CreateIdentity()) const;
    std::string     GetText     (const char* tag, std::string   defVal = "") const;

    void SetInt     (const char* tag, int   value);
    void SetBool    (const char* tag, bool  value);
    void SetFloat   (const char* tag, float value);
    void SetVec2    (const char* tag, const asdx::Vector2& value);
    void SetVec3    (const char* tag, const asdx::Vector3& value);
    void SetVec4    (const char* tag, const asdx::Vector4& value);
    void SetMatrix  (const char* tag, const asdx::Matrix&  value);
    void SetText    (const char* tag, const std::string&   value);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::map<std::string, int>              m_Int;
    std::map<std::string, bool>             m_Bool;
    std::map<std::string, float>            m_Float;
    std::map<std::string, asdx::Vector2>    m_Vec2;
    std::map<std::string, asdx::Vector3>    m_Vec3;
    std::map<std::string, asdx::Vector4>    m_Vec4;
    std::map<std::string, asdx::Matrix>     m_Matrix;
    std::map<std::string, std::string>      m_Text;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx