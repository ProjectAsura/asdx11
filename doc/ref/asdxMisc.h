//--------------------------------------------------------------------------------------------------
// File : asdxUtil.h
// Desc : Utility Module.
// Copyright(c) Project Asura. All right reserved.
//--------------------------------------------------------------------------------------------------
#pragma once

//--------------------------------------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------------------------------------
#include <asdxTypedef.h>
#include <d3d11.h>
#include <string>


namespace asdx {

//-------------------------------------------------------------------------------------------------
//! @brief      2つの値を入れ替えます.
//!
//! @param [in/out]     a           入力値.
//! @param [in/out]     b           入力値.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE_INLINE(T)
void Swap( T& a, T& b )
{
    T tmp = a;
    a     = b;
    b     = tmp;
}

//-------------------------------------------------------------------------------------------------
//! @brief      エンディアンを交換します.
//!
//! @param [in/out]     エンディアンフリップする値へのポインタ.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE_INLINE(T)
void SwapEndian( T* ptr )
{
    u8* pBegin = reinterpret_cast<u8*>( ptr );
    u8* pEnd   = reinterpret_cast<u8*>( ptr ) + sizeof( T );

    for( ; pBegin < pEnd; --pEnd )
    {
        u8 tmp      = (*pBegin);
        (*pBegin)   = *(pEnd - 1);
        *(pEnd - 1) = tmp;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      指定されたポインタ指定された数値の倍数に切り上げます.
//!
//! @param [in]     x       ポインタ
//! @param [in]     base    倍数
//! @return     x を base の倍数に切り上げた結果を返却します.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE_INLINE(T)
void*   RoundUp( T* x, u32 base )
{
    uptr value = GetIntPtr( x );
    uptr rounded = ( value + ( base - 1 ) ) & ~( base - 1 );
    return reinterpret_cast<void*>( rounded );
}

//-------------------------------------------------------------------------------------------------
//! @brief      指定されたポインタ指定された数値の倍数に切り上げます(const版).
//!
//! @param [in]     x       ポインタ
//! @param [in]     base    倍数
//! @return     x を base の倍数に切り上げた結果を返却します.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE_INLINE(T)
const void* RoundUp( const T* x, u32 base )
{
    uptr value = GetIntPtr( x );
    uptr rounded = ( value + ( base - 1 ) ) & ~( base - 1 );

    return reinterpret_cast<const void*>( rounded );
}

//-------------------------------------------------------------------------------------------------
//! @brief      指定された数値の倍数に切り上げます.
//!
//! @param [in]     val     数値.
//! @param [in]     base    倍数.
//! @return     val を base の倍数に切り上げた結果を返却します.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
u32 RoundUp( u32 val, u32 base )
{
    return ( val + ( base - 1 ) ) & ~( base - 1 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      指定された数値の倍数に切り上げます.
//!
//! @param [in]     val     数値.
//! @param [in]     base    倍数.
//! @return     val を base の倍数に切り上げた結果を返却します.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
u64 RoundUp( u64 val, u64 base )
{
    return ( val + ( base - 1 ) ) & ~( base - 1 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      指定されたポインタ指定された数値の倍数に切り下げます.
//!
//! @param [in]     x       ポインタ
//! @param [in]     base    倍数
//! @return     x を base の倍数に切り下げた結果を返却します.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE_INLINE(T)
void*   RoundDown( T* x, u32 base )
{
    uptr value = reinterpret_cast<uptr>( x );
    uptr rounded = value & ~( base - 1 );

    return reinterpret_cast<void*>( rounded );
}

//-------------------------------------------------------------------------------------------------
//! @brief      指定されたポインタ指定された数値の倍数に切り下げます(const版).
//!
//! @param [in]     x       ポインタ
//! @param [in]     base    倍数
//! @return     x を base の倍数に切り下げた結果を返却します.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE_INLINE(T)
const void* RoundDown( const T* x, u32 base )
{
    uptr value = reinterpret_cast<uptr>( x );
    uptr rounded = value & ~( base - 1 );

    return reinterpret_cast<const void*>( rounded );
}

//-------------------------------------------------------------------------------------------------
//! @brief      指定された数値の倍数に切り下げます.
//!
//! @param [in]     val     数値.
//! @param [in]     base    倍数.
//! @return     val を base の倍数に切り下げた結果を返却します.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
u32 RoundDown( u32 val, u32 base )
{
    return  val & ~( base - 1 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      指定された数値の倍数に切り下げます.
//!
//! @param [in]     val     数値.
//! @param [in]     base    倍数.
//! @return     val を base の倍数に切り下げた結果を返却します.
//-------------------------------------------------------------------------------------------------
ASDX_INLINE
u64 RoundDown( u64 val, u64 base )
{
    return  val & ~( base - 1 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      デバッグオブジェクト名を設定します.
//!
//! @param [in]     resource        デバッグオブジェクト名を設定するリソース.
//! @param [in]     name            設定するデバッグオブジェクト名.
//-------------------------------------------------------------------------------------------------
template<u32 NameLength> inline
void SetDebugObjectName( ID3D11DeviceChild* resource, const char8 (&name)[NameLength] )
{
    #if defined(_DEBUG) || defined(DEBUG)
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, NameLength - 1, name);
    #else
        UNREFERENCED_PARAMETER(resource);
        UNREFERENCED_PARAMETER(name);
    #endif
}

//-------------------------------------------------------------------------------------------------
//! @brief      バイトコードから入力レイアウトを生成します.
//!
//! @param[in]      pDevice         デバイスです.
//! @param[in]      pByteCode       バイトコードです.
//! @param[in]      size            サイズです.
//! @param[out]     ppLayout        入力レイアウトです.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
bool CreateInputLayoutFromByteCode(
    ID3D11Device*       pDevice,
    const u8*           pByteCode,
    const u32           size,
    ID3D11InputLayout** ppLayout );

//-------------------------------------------------------------------------------------------------
//! @brief      機能レベルから対応する文字列を取得します.
//!
//! @parma [in]     value       機能レベル.
//! @return     機能レベルから対応する文字列を返却します.
//-------------------------------------------------------------------------------------------------
LPSTR   GetFeatureLevelStringA( const D3D_FEATURE_LEVEL value );

//-------------------------------------------------------------------------------------------------
//! @brief      機能レベルから対応する文字列を取得します.
//!
//! @parma [in]     value       機能レベル.
//! @return     機能レベルから対応する文字列を返却します.
//-------------------------------------------------------------------------------------------------
LPWSTR  GetFeatureLevelStringW( const D3D_FEATURE_LEVEL value );

//-------------------------------------------------------------------------------------------------
//! @brief      ドライバータイプから対応する文字列を取得します.
//!
//! @param [in]     value           ドライバータイプ.
//! @return     ドライバータイプから対応する文字列を返却します.
//-------------------------------------------------------------------------------------------------
LPSTR   GetDriverTypeStringA( const D3D_DRIVER_TYPE value );

//-------------------------------------------------------------------------------------------------
//! @brief      ドライバータイプから対応する文字列を取得します.
//!
//! @param [in]     value           ドライバータイプ.
//! @return     ドライバータイプから対応する文字列を返却します.
//-------------------------------------------------------------------------------------------------
LPWSTR  GetDriverTypeStringW( const D3D_DRIVER_TYPE value );

//-------------------------------------------------------------------------------------------------
//! @brief      DXGIフォーマットから対応する文字列を取得します.
//!
//! @param [in]     value           DXGIフォーマット.
//! @return     DXGIフォーマットから対応する文字列を返却します.
//-------------------------------------------------------------------------------------------------
LPSTR   GetFormatStringA( const DXGI_FORMAT value );

//-------------------------------------------------------------------------------------------------
//! @brief      DXGIフォーマットから対応する文字列を取得します.
//!
//! @param [in]     value           DXGIフォーマット.
//! @return     DXGIフォーマットから対応する文字列を返却します.
//-------------------------------------------------------------------------------------------------
LPWSTR  GetFormatStringW( const DXGI_FORMAT value );

#if defined(UNICODE) || defined(_UNICODE)
    #define GetFeatureLevelString       GetFeatureLevelStringW
    #define GetDriverTypeString         GetDriverTypeStringW
    #define GetFormatString             GetFormatStringW
#else
    #define GetFeatureLevelString       GetFeatureLevelStringA
    #define GetDriverTypeString         GetDriverTypeStringA
    #define GetFormatString             GetFormatStringA
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      DXGIフォーマットから1ピクセルあたりのビット数を取得します.
//
//! @param [in]     format          DXGIフォーマット.
//! @return     1ピクセルあたりのビット数を返却します.
//-------------------------------------------------------------------------------------------------
size_t  GetBitsPerPixel( DXGI_FORMAT format );

//-------------------------------------------------------------------------------------------------
//! @brief      サーフェイス情報を取得します.
//!
//! @param [in]     width           画像の横幅.
//! @param [in]     height          画像の縦幅.
//! @param [in]     format          DXGIフォーマット.
//! @param [out]    outNumBytes     バイト数.
//! @param [out]    outRowBytes     1列あたりのバイト数.
//! @param [out]    outNumRows      列数.
//-------------------------------------------------------------------------------------------------
void    GetSurfaceInfo( size_t width, size_t height, DXGI_FORMAT format, size_t* outNumBytes, size_t* outRowBytes, size_t* outNumRows );

//-------------------------------------------------------------------------------------------------
//! @brief      スクリーンキャプチャーを行います.
//!
//! @param [in]     pDeviceContext      デバイスコンテキストです.
//! @param [in]     pSwapChain          スワップチェインです.
//! @param [out]    ppTexture           キャプチャーイメージを格納するテクスチャです.
//! @retval true    キャプチャーに成功.
//! @retval false   キャプチャーに失敗.
//-------------------------------------------------------------------------------------------------
bool ScreenCapture( ID3D11DeviceContext* pDeviceContext, IDXGISwapChain* pSwapChain, ID3D11Texture2D** ppTexture );

//-------------------------------------------------------------------------------------------------
//! @brief      スクリーンキャプチャーを行います.
//!
//! @param [in]     pDeviceContext      デバイスコンテキストです.
//! @param [in]     pBuffer             入力リソースです.
//! @param [out]    ppTexture           キャプチャーイメージを格納するテクスチャです.
//! @retval true    キャプチャーに成功.
//! @retval false   キャプチャーに失敗.
//-------------------------------------------------------------------------------------------------
bool ScreenCapture( ID3D11DeviceContext* pDeviceContext, ID3D11Texture2D* pBuffer, ID3D11Texture2D** ppTexture );

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャをBMPファイルとして保存します.
//!
//! @param [in]     pDeviceContext      デバイスコンテキストです.
//! @param [in]     pTexture            テクスチャです.
//! @param [in]     fileName            出力ファイル名です.
//! @retval true    保存に成功.
//! @retval false   保存に失敗.
//-------------------------------------------------------------------------------------------------
bool SaveTextureToBmpA( ID3D11DeviceContext* pDeviceContext, ID3D11Texture2D* pTexture, const char*    fileName );

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャをBMPファイルとして保存します.
//!
//! @param [in]     pDeviceContext      デバイスコンテキストです.
//! @param [in]     pTexture            テクスチャです.
//! @param [in]     fileName            出力ファイル名です.
//! @retval true    保存に成功.
//! @retval false   保存に失敗.
//-------------------------------------------------------------------------------------------------
bool SaveTextureToBmpW( ID3D11DeviceContext* pDeviceContext, ID3D11Texture2D* pTexture, const wchar_t* fileName );

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャをTGAファイルとして保存します.
//!
//! @param [in]     pDeviceContext      デバイスコンテキストです.
//! @param [in]     pTexture            テクスチャです.
//! @param [in]     fileName            出力ファイル名です.
//! @retval true    保存に成功.
//! @retval false   保存に失敗.
//-------------------------------------------------------------------------------------------------
bool SaveTextureToTgaA( ID3D11DeviceContext* pDeviceContext, ID3D11Texture2D* pTexture, const char*    filename );

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャをTGAファイルとして保存します.
//!
//! @param [in]     pDeviceContext      デバイスコンテキストです.
//! @param [in]     pTexture            テクスチャです.
//! @param [in]     fileName            出力ファイル名です.
//! @retval true    保存に成功.
//! @retval false   保存に失敗.
//-------------------------------------------------------------------------------------------------
bool SaveTextureToTgaW( ID3D11DeviceContext* pDeviceContext, ID3D11Texture2D* pTexture, const wchar_t* filename );

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャをBMPファイルとして保存します.
//!
//! @param [in]     fileName            出力ファイル名です.
//! @param [in]     width               テクスチャの横幅です.
//! @param [in]     height              テクスチャの縦幅です.
//! @param [in]     component           ピクセルを構成するチャンネル数です(RGB=3, RGBA=4).
//! @retval true    保存に成功.
//! @retval false   保存に失敗.
//-------------------------------------------------------------------------------------------------
bool SaveTextureToBmpA( const char*    filename, const int width, const int height, const int component, const unsigned char* pPixels );

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャをBMPファイルとして保存します.
//!
//! @param [in]     fileName            出力ファイル名です.
//! @param [in]     width               テクスチャの横幅です.
//! @param [in]     height              テクスチャの縦幅です.
//! @param [in]     component           ピクセルを構成するチャンネル数です(RGB=3, RGBA=4).
//! @retval true    保存に成功.
//! @retval false   保存に失敗.
//-------------------------------------------------------------------------------------------------
bool SaveTextureToBmpW( const wchar_t* filename, const int width, const int height, const int component, const unsigned char* pPixels );

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャをTGAファイルとして保存します.
//!
//! @param [in]     fileName            出力ファイル名です.
//! @param [in]     width               テクスチャの横幅です.
//! @param [in]     height              テクスチャの縦幅です.
//! @param [in]     component           ピクセルを構成するチャンネル数です(RGB=3, RGBA=4).
//! @retval true    保存に成功.
//! @retval false   保存に失敗.
//-------------------------------------------------------------------------------------------------
bool SaveTextureToTgaA( const char*    filename, const int width, const int height, const int component, const unsigned char* pPixels );

//-------------------------------------------------------------------------------------------------
//! @brief      テクスチャをTGAファイルとして保存します.
//!
//! @param [in]     fileName            出力ファイル名です.
//! @param [in]     width               テクスチャの横幅です.
//! @param [in]     height              テクスチャの縦幅です.
//! @param [in]     component           ピクセルを構成するチャンネル数です(RGB=3, RGBA=4).
//! @retval true    保存に成功.
//! @retval false   保存に失敗.
//-------------------------------------------------------------------------------------------------
bool SaveTextureToTgaW( const wchar_t* filename, const int width, const int height, const int component, const unsigned char* pPixels );

//-------------------------------------------------------------------------------------------------
//! @brief      ファイルパスを検索します.
//!
//! @param[in]      filePath        検索するファイスパス.
//! @param[out]     result          検索結果の格納先.
//! @retval true    ファイルを発見.
//! @retval false   ファイルが見つからなかった.
//! @memo 検索ルールは以下の通り.
//!      .\
//!      ..\
//!      ..\..\
//!      .\res\
//!      %EXE_DIR%\
//!      %EXE_DIR%\..\
//!      %EXE_DIR%\..\..\
//!      %EXE_DIR%\res\
//-------------------------------------------------------------------------------------------------
bool SearchFilePath( const char16* filePath, std::wstring& result );

//-------------------------------------------------------------------------------------------------
//! @brief      ディレクトリ名を取得します.
//!
//! @param[in]      filePath        ファイルパス.
//! @return     指定されたファイルパスからディレクトリ名を抜き出します.
//-------------------------------------------------------------------------------------------------
std::wstring GetDirectoryPath( const char16* filePath );

//-------------------------------------------------------------------------------------------------
//! @brief      拡張子を取得します.
//!
//! @param[in]      filePath        ファイルパス.
//! @return     指定されたファイルパスから拡張子を抜き出します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExt( const char16* filePath );

//-------------------------------------------------------------------------------------------------
//! @brief      実行ファイルのファイルパスを取得します.
//!
//! @return     実行ファイルのファイルパスを返却します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExePath();

//-------------------------------------------------------------------------------------------------
//! @brief      指定されたファイルパスが存在するかチェックします.
//!
//! @param[in]      filePath        チェックするファイルパス.
//! @return     指定されたファイルパスが存在する場合はtrueを返却します.
//-------------------------------------------------------------------------------------------------
bool CheckFilePath( const char16* filePath );

//-------------------------------------------------------------------------------------------------
//! @brief      指定されたフォルダパスが存在するかチェックします.
//!
//! @param[in]      folderPath      チェックするフォルダパス.
//! @return     指定されたフォルダパスが存在する場合はtrueを返却します.
//-------------------------------------------------------------------------------------------------
bool CheckFolderPath( const char16* folderPath );

//-------------------------------------------------------------------------------------------------
//! @brief      文字列から最末尾のディレクトリを削ぎ落とします.
//!
//! @return     削ぎ落としに成功したらtrueを返却します.
//-------------------------------------------------------------------------------------------------
bool CutLastDirectoryPath( char16* path );

//-------------------------------------------------------------------------------------------------
//! @brief      時間を表す文字列を取得します.
//!
//! @return     yyyyMMdd_hhmmssの形式で文字列を返却します.
//-------------------------------------------------------------------------------------------------
std::wstring GetTimeString();


} // namespace asdx
