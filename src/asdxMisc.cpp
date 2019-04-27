//-------------------------------------------------------------------------------------------------
// File : asdxMisc.cpp
// Desc : Utility Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxMisc.h>
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <shlwapi.h>
#include <locale>
#include <codecvt>
#include <sstream>


namespace asdx {

//-------------------------------------------------------------------------------------------------
//      ファイルパスを検索します.
//-------------------------------------------------------------------------------------------------
bool SearchFilePathW( const wchar_t* filePath, std::wstring& result )
{
    if ( filePath == nullptr )
    { return false; }

    if ( wcscmp( filePath, L" " ) == 0 || wcscmp( filePath, L"" ) == 0 )
    { return false; }

    wchar_t exePath[ 520 ] = { 0 };
    GetModuleFileNameW( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.
    PathRemoveFileSpecW( exePath );

    wchar_t dstPath[ 520 ] = { 0 };

    wcscpy_s( dstPath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"..\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"..\\..\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"\\res\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\..\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\..\\..\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\res\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスを検索します.
//-------------------------------------------------------------------------------------------------
bool SearchFilePathA( const char* filePath, std::string& result )
{
    if ( filePath == nullptr )
    { return false; }

    if ( strcmp( filePath, " " ) == 0 || strcmp( filePath, "" ) == 0 )
    { return false; }

    char exePath[ 520 ] = { 0 };
    GetModuleFileNameA( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.
    PathRemoveFileSpecA( exePath );

    char dstPath[ 520 ] = { 0 };

    strcpy_s( dstPath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "..\\%s", filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "..\\..\\%s", filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "\\res\\%s", filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\..\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\..\\..\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\res\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスからディレクトリ名を取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetDirectoryPathW( const wchar_t* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"/" );
    if ( idx != std::wstring::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    idx = path.find_last_of( L"\\" );
    if ( idx != std::wstring::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    return std::wstring();
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスからディレクトリ名を取得します.
//-------------------------------------------------------------------------------------------------
std::string GetDirectoryPathA( const char* filePath )
{
    std::string path = filePath;
    auto idx = path.find_last_of( "/" );
    if ( idx != std::string::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    idx = path.find_last_of( "\\" );
    if ( idx != std::string::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    return std::string();
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスからディレクトリ名を削除します.
//-------------------------------------------------------------------------------------------------
std::wstring RemoveDirectoryPathW( const wchar_t* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"/" );
    if ( idx != std::wstring::npos )
    {
        auto result = path.substr( idx + 1 );
        return result;
    }

    idx = path.find_last_of( L"\\" );
    if ( idx != std::wstring::npos )
    {
        auto result = path.substr( idx + 1 );
        return result;
    }

    return path;
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスからディレクトリ名を削除します.
//-------------------------------------------------------------------------------------------------
std::string RemoveDirectoryPathA( const char* filePath )
{
    std::string path = filePath;
    auto idx = path.find_last_of( "/" );
    if ( idx != std::string::npos )
    {
        auto result = path.substr( idx + 1 );
        return result;
    }

    idx = path.find_last_of( "\\" );
    if ( idx != std::string::npos )
    {
        auto result = path.substr( idx + 1 );
        return result;
    }

    return path;
}

//-------------------------------------------------------------------------------------------------
//      実行ファイルのファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExePathW()
{
    wchar_t exePath[ 520 ] = { 0 };
    GetModuleFileNameW( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.

    return asdx::GetDirectoryPathW( exePath );
}

//-------------------------------------------------------------------------------------------------
//      実行ファイルのファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::string GetExePathA()
{
    char exePath[ 520 ] = { 0 };
    GetModuleFileNameA( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.

    return asdx::GetDirectoryPathA( exePath );
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスから拡張子を取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExtW( const wchar_t* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"." );
    if ( idx != std::wstring::npos )
    {
        std::wstring result = path.substr( idx + 1 );

        // 小文字化.
        std::transform( result.begin(), result.end(), result.begin(), tolower );

        return result;
    }

    return std::wstring();
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスから拡張子を取得します.
//-------------------------------------------------------------------------------------------------
std::string GetExtA( const char* filePath )
{
    std::string path = filePath;
    auto idx = path.find_last_of( "." );
    if ( idx != std::string::npos )
    {
        std::string result = path.substr( idx + 1 );

        // 小文字化.
        std::transform( result.begin(), result.end(), result.begin(), tolower );

        return result;
    }

    return std::string();
}


//-------------------------------------------------------------------------------------------------
//      拡張子を取り除いたファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetPathWithoutExtW( const wchar_t* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"." );
    if ( idx != std::wstring::npos )
    {
        return path.substr( 0, idx );
    }

    return path;
}

//-------------------------------------------------------------------------------------------------
//      拡張子を取り除いたファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::string GetPathWithoutExtA( const char* filePath )
{
    std::string path = filePath;
    auto idx = path.find_last_of( "." );
    if ( idx != std::string::npos )
    {
        return path.substr( 0, idx );
    }

    return path;
}


//-------------------------------------------------------------------------------------------------
//      指定されたファイルパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFilePathW( const wchar_t* filePath )
{
    if ( PathFileExistsW( filePath ) == TRUE )
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      指定されたファイルパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFilePathA( const char* filePath )
{
    if ( PathFileExistsA( filePath ) == TRUE )
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      指定されたフォルダパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFolderPathA( const char* folderPath )
{
    if ( PathFileExistsA ( folderPath ) == TRUE
      && PathIsDirectoryA( folderPath ) != FALSE ) // PathIsDirectoryA() は TRUE を返却しないので注意!!
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      指定されたフォルダパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFolderPathW( const wchar_t* folderPath )
{
    if ( PathFileExistsW ( folderPath ) == TRUE
      && PathIsDirectoryW( folderPath ) != FALSE ) // PathIsDirectoryW() は TRUE を返却しないので注意!!
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      ワイド文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::wstring ToStringW( const std::string& value )
{
    auto length = MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1), nullptr, 0 );
    auto buffer = new wchar_t[length];

    MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1),  buffer, length );

    std::wstring result( buffer );
    delete[] buffer;

    return result;
}

//-------------------------------------------------------------------------------------------------
//      マルチバイト文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::string ToStringA( const std::wstring& value )
{
    auto length = WideCharToMultiByte(CP_ACP, 0, value.c_str(), int(value.size() + 1), nullptr, 0, nullptr, nullptr); 
    auto buffer = new char[length];
 
    WideCharToMultiByte(CP_ACP, 0, value.c_str(), int(value.size() + 1), buffer, length, nullptr, nullptr);

    std::string result(buffer);
    delete[] buffer;

    return result;
}

//-------------------------------------------------------------------------------------------------
//      UTF-8文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::string ToStringUTF8(const std::wstring& src)
{
    auto const dest_size = ::WideCharToMultiByte(CP_UTF8, 0U, src.data(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> dest(dest_size, '\0');
    if (::WideCharToMultiByte(CP_UTF8, 0U, src.data(), -1, dest.data(), int(dest.size()), nullptr, nullptr) == 0) {
        throw std::system_error{static_cast<int>(::GetLastError()), std::system_category()};
    }
    return std::string(dest.begin(), dest.end());
}

//-------------------------------------------------------------------------------------------------
//      UTF-8文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::string ToStringUTF8(const std::string& value)
{
    auto wide = ToStringW(value);
    return ToStringUTF8(wide);
}

//-------------------------------------------------------------------------------------------------
//      指定文字で文字列を分割します.
//-------------------------------------------------------------------------------------------------
std::vector<std::string> Split(const std::string& input, char delimiter)
{
    std::istringstream stream(input);

    std::string field;
    std::vector<std::string> result;
    while (std::getline(stream, field, delimiter))
    { result.push_back(field); }
    return result;
}

//-------------------------------------------------------------------------------------------------
//      指定文字で文字列を分割します.
//-------------------------------------------------------------------------------------------------
std::vector<std::wstring> Split(const std::wstring& input, wchar_t delimiter)
{
    std::wistringstream stream(input);

    std::wstring field;
    std::vector<std::wstring> result;
    while (std::getline(stream, field, delimiter))
    { result.push_back(field); }
    return result;
}

//-----------------------------------------------------------------------------
//      外部プロセスを実行します.
//-----------------------------------------------------------------------------
bool RunProcess(const char* cmd, bool wait)
{
    STARTUPINFOA        startup_info = {};
    PROCESS_INFORMATION process_info = {};

    DWORD flag = NORMAL_PRIORITY_CLASS;
    startup_info.cb = sizeof(STARTUPINFOA);

    // 成功すると0以外, 失敗すると0が返る.
    auto ret = CreateProcessA(
        nullptr,
        const_cast<char*>(cmd), // 実害はないはず...
        nullptr,
        nullptr,
        FALSE,
        flag,
        nullptr,
        nullptr,
        &startup_info,
        &process_info);

    if (ret == 0)
    {
        //ELOGA("Error : プロセス起動に失敗. コマンド = %s", cmd);
        CloseHandle(process_info.hProcess);
        CloseHandle(process_info.hThread);
        return false;
    }

    if (wait)
    { WaitForSingleObject(process_info.hProcess, INFINITE); }

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    return true;
}

} // namespace asdx
