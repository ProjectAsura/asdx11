//-----------------------------------------------------------------------------
// File : asdxFlatDoc.cpp
// Desc : Flat Document.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxFlatDoc.h>
#include <fstream>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// FlatDoc class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
FlatDoc::FlatDoc()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
FlatDoc::~FlatDoc()
{
    m_Int   .clear();
    m_Bool  .clear();
    m_Float .clear();
    m_Vec2  .clear();
    m_Vec3  .clear();
    m_Vec4  .clear();
    m_Text  .clear();
}

//-----------------------------------------------------------------------------
//      ファイルから読み込みます.
//-----------------------------------------------------------------------------
bool FlatDoc::Load(const char* path)
{
    std::ifstream file;

    // ファイルを開く.
    file.open( path, std::ios::in );

    if (!file.is_open())
    { return false; }

    char buf[ 2048 ];

    for(;;)
    {
        // バッファに格納.
        file >> buf;

        // ファイル末端かチェック
        if ( !file )
            break;

        if ( file.eof() )
            break;

        //　コメント
        if ( 0 == strcmp( buf, "#" ) )
        { /* DO_NOTHING */ }
        else if ( 0 == strcmp( buf, "int") )
        {
            std::string tag;
            int value;
            file >> tag >> value;
            m_Int[tag] = value;
        }
        else if ( 0 == strcmp( buf, "bool") )
        {
            std::string tag;
            bool value;
            file >> tag >> value;
            m_Bool[tag] = value;
        }
        else if ( 0 == strcmp( buf, "float") )
        {
            std::string tag;
            float value;
            file >> tag >> value;
            m_Float[tag] = value;
        }
        else if ( 0 == strcmp( buf, "vec2") )
        {
            std::string tag;
            float x, y;
            file >> tag >> x >> y;
            m_Vec2[tag] = asdx::Vector2(x, y);
        }
        else if ( 0 == strcmp( buf, "vec3") )
        {
            std::string tag;
            float x, y, z;
            file >> tag >> x >> y >> z;
            m_Vec3[tag] = asdx::Vector3(x, y, z);
        }
        else if ( 0 == strcmp( buf, "vec4") )
        {
            std::string tag;
            float x, y, z, w;
            file >> tag >> x >> y >> z >> w;
            m_Vec4[tag] = asdx::Vector4(x, y, z, w);
        }
        else if ( 0 == strcmp( buf, "string") )
        {
            std::string tag;
            std::string value;
            file >> tag >> value;
            m_Text[tag] = value;
        }

        file.ignore( 2048, '\n' );
    }

    file.close();

    return true;
}

//-----------------------------------------------------------------------------
//      ファイルに保存します.
//-----------------------------------------------------------------------------
bool FlatDoc::Save(const char* path)
{
    FILE* pFile;

    auto ret = fopen_s(&pFile, path, "wb");
    if (ret != 0)
    { return false; }

    for(auto& itr : m_Int)
    { fprintf_s(pFile, "int %s %d\n", itr.first.c_str(), itr.second); }

    for(auto& itr : m_Bool)
    { fprintf_s(pFile, "bool %s %d\n", itr.first.c_str(), itr.second); }

    for(auto& itr : m_Float)
    { fprintf_s(pFile, "float %s %f\n", itr.first.c_str(), itr.second); }

    for(auto& itr : m_Vec2)
    { fprintf_s(pFile, "vec2 %s %f %f\n", itr.first.c_str(), itr.second.x, itr.second.y); }

    for(auto& itr : m_Vec3)
    { fprintf_s(pFile, "vec3 %s %f %f %f\n", itr.first.c_str(), itr.second.x, itr.second.y, itr.second.z); }

    for(auto& itr : m_Vec4)
    { fprintf_s(pFile, "vec4 %s %f %f %f %f\n", itr.first.c_str(), itr.second.x, itr.second.y, itr.second.z, itr.second.w); }

    for(auto& itr : m_Text)
    { fprintf_s(pFile, "string %s %s\n", itr.first.c_str(), itr.second.c_str()); }

    fclose(pFile);
    return true;
}

//-----------------------------------------------------------------------------
//      整数値を取得します.
//-----------------------------------------------------------------------------
int FlatDoc::GetInt(const char* tag, int defVal) const
{
    if (m_Int.find(tag) != std::end(m_Int))
    { return m_Int.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      ブール値を取得します.
//-----------------------------------------------------------------------------
bool FlatDoc::GetBool(const char* tag, bool defVal) const
{
    if (m_Bool.find(tag) != std::end(m_Bool))
    { return m_Bool.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      浮動小数値を取得します.
//-----------------------------------------------------------------------------
float FlatDoc::GetFloat(const char* tag, float defVal) const
{
    if (m_Float.find(tag) != std::end(m_Float))
    { return m_Float.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      2次元ベクトルを取得します.
//-----------------------------------------------------------------------------
asdx::Vector2 FlatDoc::GetVec2(const char* tag, asdx::Vector2 defVal) const
{
    if (m_Vec2.find(tag) != std::end(m_Vec2))
    { return m_Vec2.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      3次元ベクトルを取得します.
//-----------------------------------------------------------------------------
asdx::Vector3 FlatDoc::GetVec3(const char* tag, asdx::Vector3 defVal) const
{
    if (m_Vec3.find(tag) != std::end(m_Vec3))
    { return m_Vec3.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      4次元ベクトルを取得します.
//-----------------------------------------------------------------------------
asdx::Vector4 FlatDoc::GetVec4(const char* tag, asdx::Vector4 defVal) const
{
    if (m_Vec4.find(tag) != std::end(m_Vec4))
    { return m_Vec4.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      テキストを取得します.
//-----------------------------------------------------------------------------
std::string FlatDoc::GetText(const char* tag, std::string defVal) const
{
    if (m_Text.find(tag) != std::end(m_Text))
    { return m_Text.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      整数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetInt(const char* tag, int value)
{ m_Int[tag] = value; }

//-----------------------------------------------------------------------------
//      ブール値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetBool(const char* tag, bool value)
{ m_Bool[tag] = value; }

//-----------------------------------------------------------------------------
//      浮動小数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetFloat(const char* tag, float value)
{ m_Float[tag] = value; }

//-----------------------------------------------------------------------------
//      2次元ベクトルを設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetVec2(const char* tag, const asdx::Vector2& value)
{ m_Vec2[tag] = value; }

//-----------------------------------------------------------------------------
//      3次元ベクトルを設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetVec3(const char* tag, const asdx::Vector3& value)
{ m_Vec3[tag] = value; }

//-----------------------------------------------------------------------------
//      4次元ベクトルを設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetVec4(const char* tag, const asdx::Vector4& value)
{ m_Vec4[tag] = value; }

//-----------------------------------------------------------------------------
//      テキストを設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetText(const char* tag, const std::string& value)
{ m_Text[tag] = value; }

} // namespace asdx