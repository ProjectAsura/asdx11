//-----------------------------------------------------------------------------
// File : asdxIncludeExpansion.cpp
// Desc : Include Expansion.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxIncludeExpansion.h>
#include <cstdio>
#include <fstream>
#include <locale>
#include <codecvt>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// IncludeExpansion class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
IncludeExpansion::IncludeExpansion()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
IncludeExpansion::~IncludeExpansion()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool IncludeExpansion::Init
(
    const char*                     filename,
    const std::vector<std::string>& dirPaths
)
{
    m_DirPaths = dirPaths;
    if (!CorrectInfo(filename))
    { return false; }

    for (size_t i = 0; i < m_Includes.size(); ++i)
    { LoadFile(m_Includes[i].FindPath.c_str(), m_Includes[i].Code); }

    if (!LoadFile(filename, m_Expanded))
    { return false; }

    for (;;)
    {
        if (!Expand(m_Expanded))
        { break; }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void IncludeExpansion::Term()
{
    m_Includes.clear();
    m_DirPaths.clear();
    m_Expanded.clear();
}

//-----------------------------------------------------------------------------
//      展開済み結果を取得します.
//-----------------------------------------------------------------------------
const std::string& IncludeExpansion::GetExpandResult() const
{ return m_Expanded; }

//-----------------------------------------------------------------------------
//      ファイルをロードします.
//-----------------------------------------------------------------------------
bool IncludeExpansion::LoadFile(const char* filename, std::string& result)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, filename, "rb");
    if (err != 0)
    { return false; }

    auto cur_pos = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    auto end_pos = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    auto size = end_pos - cur_pos;
    auto pBuffer = new char[size + 1];
    fread(pBuffer, size, 1, pFile);
    pBuffer[size] = '\0';
    fclose(pFile);

    result = pBuffer;
    delete[] pBuffer;

    return true;
}

//-----------------------------------------------------------------------------
//      必要なインクルードファイル情報をかき集めます.
//-----------------------------------------------------------------------------
bool IncludeExpansion::CorrectInfo(const char* filename)
{
    std::ifstream stream;
    stream.open(filename, std::ios::in);

    if (!stream.is_open())
    { return false; }

    std::string line;

    for (;;)
    {
        if (!stream)
        { break; }

        if (stream.eof())
        { break; }

        stream >> line;

        auto pos = line.find("#include");
        if (pos != std::string::npos)
        {
            stream >> line;

            Info info;
            info.IncludeFile = "#include ";
            info.IncludeFile += line;

            line = Replace(line, "\"", "");
            line = Replace(line, "<", "");
            line = Replace(line, ">", "");
            line = Replace(line, "\r\n", "");

            for (size_t i = 0; i < m_DirPaths.size(); ++i)
            {
                auto path = m_DirPaths[i] + "\\" + line;
                if (CorrectInfo(path.c_str()))
                {
                    info.FindPath = path;
                    break;
                }
            }

            // ここで追加.
            m_Includes.push_back(info);
        }
    }

    stream.close();
    return true;
}

//-----------------------------------------------------------------------------
//      文字列を置換します.
//-----------------------------------------------------------------------------
std::string IncludeExpansion::Replace
(
    const std::string&  input,
    std::string         pattern,
    std::string         replace,
    bool&               hit
)
{
    std::string result = input;
    auto pos = result.find(pattern);
    hit = false;

    while (pos != std::string::npos)
    {
        result.replace(pos, pattern.length(), replace);
        pos = result.find(pattern, pos + replace.length());
        hit = true;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      文字列を置換します.
//-----------------------------------------------------------------------------
std::string IncludeExpansion::Replace
(
    const std::string&  input,
    std::string         pattern,
    std::string         replace
)
{
    std::string result = input;
    auto pos = result.find(pattern);

    while (pos != std::string::npos)
    {
        result.replace(pos, pattern.length(), replace);
        pos = result.find(pattern, pos + replace.length());
    }

    return result;
}

//-----------------------------------------------------------------------------
//      インクルード文を展開します.
//-----------------------------------------------------------------------------
bool IncludeExpansion::Expand(std::string& inout)
{
    bool find = false;

    for (size_t i = 0; i < m_Includes.size(); ++i)
    {
        bool hit = false;
        inout = Replace(
            inout,
            m_Includes[i].IncludeFile,
            m_Includes[i].Code,
            hit);
        if (hit)
        { find = true; }
    }

    inout = Replace(inout, "\r\n", "\n");
    return find;
}

} // namespace asdx