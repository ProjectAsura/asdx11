//-------------------------------------------------------------------------------------------------
// File : asdxFont.cpp
// Desc : Font Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxFont.h>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <asdxLogger.h>
#include <asdxMisc.h>
#include <asdxRenderState.h>
#include <vector>


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// FntFormat enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum FntFormat
{
    FNT_FORMAT_R8G8B8A8 = 0,        //!< RGBA(8,8,8,8)フォーマットです.
    FNT_FORMAT_R4G4B4A4,            //!< RGBA(4,4,4,4)フォーマットです.
    FNT_FORMAT_BC2                  //!< BC2_UNORMフォーマットです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FntTextureHeader structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct FntTextureHeader
{
    uint32_t    Width;          //!< 横幅です.
    uint32_t    Height;         //!< 縦幅です.
    uint32_t    Format;         //!< フォーマットです.
    uint32_t    Stride;         //!< ストライドです(=1行あたりのデータのバイト数です).
    uint32_t    Rows;           //!< 行数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FntDataHeader structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct FntDataHeader
{
    wchar_t     FontName[ 32 ];     //!< フォント名です.
    uint32_t    FontWidth;          //!< フォントの横幅です.
    uint32_t    FontHeight;         //!< フォントの縦幅です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FntFileHeader structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct FntFileHeader
{
    uint8_t             Magic[ 4 ];         //!< マジックです.
    uint32_t            Version;            //!< ファイルバージョンです.
    uint32_t            DataHeaderSize;     //!< データヘッダ構造体のサイズです.
    uint32_t            TextureHeaderSize;  //!< テクスチャヘッダ構造体のサイズです.
    FntDataHeader       DataHeader;         //!< データヘッダです.
    FntTextureHeader    TextureHeader;      //!< テクスチャヘッダです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// CbPerOnceVS structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct CbPerOnceVS
{
    asdx::Matrix    Transform;
};

//　現在のフォントファイルバージョンです.
static const uint32_t  FNT_CURRENT_FILE_VERSION         = 0x00000002;


} // namespace /* anonymous */


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Font class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//          コンストラクタです.
//-------------------------------------------------------------------------------------------------
Font::Font()
: m_pTexture    ( nullptr )
, m_pSRV        ( nullptr )
, m_FontWidth   ( 0 )
, m_FontHeight  ( 0 )
, m_OffsetU     ( 0.0f )
, m_OffsetV     ( 0.0f )
{
    m_FontName[0] = '\0';
}

//-------------------------------------------------------------------------------------------------
//          デストラクタです.
//-------------------------------------------------------------------------------------------------
Font::~Font()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//          初期化処理です.
//-------------------------------------------------------------------------------------------------
bool Font::Init
(
    ID3D11Device*   pDevice,
    const wchar_t*   filename
)
{
    // レンダーステートを初期化.
    if ( !RenderState::GetInstance().Init( pDevice ) )
    {
        ELOG( "Error : RenderState Initialize Failed." );
        return false;
    }

    FILE* pFile;

    // ファイルを開きます.
    errno_t err = _wfopen_s( &pFile, filename, L"rb" );

    // チェック.
    if ( err != 0 )
    {
        // エラーログ出力.
        ELOGW( "Error : File Open Failed. filename = %s\n", filename );

        // 異常終了.
        return false;
    }

    // ファイルヘッダ読み込み.
    FntFileHeader fileHeader;
    fread( &fileHeader, sizeof(FntFileHeader), 1, pFile );

    // ファイルマジックをチェック.
    if ( ( fileHeader.Magic[0] != 'F' )
      || ( fileHeader.Magic[1] != 'N' )
      || ( fileHeader.Magic[2] != 'T' )
      || ( fileHeader.Magic[3] != '\0' ) )
    {
        // エラーログ出力.
        ELOGW( "Error : Invalid File. filename = %s\n", filename );

        // ファイルを閉じる.
        fclose( pFile );

        // 異常終了.
        return false;
    }

    // ファイルバージョンのチェック.
    if ( fileHeader.Version != FNT_CURRENT_FILE_VERSION )
    {
        // エラーログ出力.
        ELOG( "Error : Invalid File Version. version = 0x%x, current_version = 0x%x\n", fileHeader.Version, FNT_CURRENT_FILE_VERSION );

        // ファイルを閉じる.
        fclose( pFile );

        // 異常終了.
        return false;
    }

    // データヘッダ構造体サイズをチェック.
    if ( fileHeader.DataHeaderSize != sizeof( FntDataHeader ) )
    {
        // エラーログ出力.
        ELOG( "Error : Invalid Data Header Size.  size = %d, expect_size = %d", fileHeader.DataHeaderSize, sizeof(FntDataHeader) );

        // ファイルを閉じる.
        fclose( pFile );

        // 異常終了.
        return false;
    }

    // テクスチャヘッダ構造体サイズをチェック.
    if ( fileHeader.TextureHeaderSize != sizeof( FntTextureHeader ) )
    {
        // エラーログ出力.
        ELOG( "Error : Invalid Texture Header Size. size = %d, expect_size = %d", fileHeader.TextureHeaderSize, sizeof(FntTextureHeader) );

        // ファイルを閉じる.
        fclose( pFile );

        // 異常終了.
        return false;
    }

    // フォントサイズを設定.
    m_FontWidth  = fileHeader.DataHeader.FontWidth;
    m_FontHeight = fileHeader.DataHeader.FontHeight;

    // フォント名を設定.
    wcscpy_s( m_FontName, fileHeader.DataHeader.FontName );

    // テクスチャ座標オフセットを算出.
    m_OffsetU = static_cast<float>( m_FontWidth )  / static_cast<float>( fileHeader.TextureHeader.Width );
    m_OffsetV = static_cast<float>( m_FontHeight ) / static_cast<float>( fileHeader.TextureHeader.Height );


    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

    // フォーマット判定.
    switch( fileHeader.TextureHeader.Format )
    {
    case FNT_FORMAT_R8G8B8A8:
        format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;

    case FNT_FORMAT_R4G4B4A4:
        format = DXGI_FORMAT_B4G4R4A4_UNORM;
        break;

    case FNT_FORMAT_BC2:
        format = DXGI_FORMAT_BC2_UNORM;
        break;

    default:
        assert( false );
        break;
    }

    HRESULT hr = S_OK;
    {
        // テクスチャサイズ算出.
        size_t textureSize = fileHeader.TextureHeader.Stride * fileHeader.TextureHeader.Rows;

        // テクセルのメモリを確保.
        uint8_t* pPixels = new uint8_t [ textureSize ];

        // NULLチェック.
        if ( pPixels == nullptr )
        {
            // エラーログ出力.
            ELOG( "Error : Memory Allocate Failed." );

            // ファイルを閉じる.
            fclose( pFile );

            // 異常終了.
            return false;
        }

        // テクセルデータを一気に読み込み.
        fread( pPixels, sizeof( uint8_t ), textureSize, pFile );

        // サブリソースデータを設定.
        D3D11_SUBRESOURCE_DATA subRes;
        subRes.pSysMem          = pPixels;
        subRes.SysMemPitch      = fileHeader.TextureHeader.Stride;
        subRes.SysMemSlicePitch = UINT( textureSize );

        // 2Dテクスチャの設定.
        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = fileHeader.TextureHeader.Width;
        desc.Height             = fileHeader.TextureHeader.Height;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = format;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        // テクスチャ生成.
        ID3D11Texture2D* pTexture = nullptr;
        hr = pDevice->CreateTexture2D( &desc, &subRes, &pTexture );

        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateTexture2D() Failed." );

            fclose( pFile );

            delete [] pPixels;
            pPixels = nullptr;

            return false;
        }

        // テクスチャを設定.
        m_pTexture.Attach( pTexture );

        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pTexture.GetPtr(), "asdx::Font::m_pTexture" );

        {
            // シェーダリソースビューの設定.
            D3D11_SHADER_RESOURCE_VIEW_DESC sd;
            ZeroMemory( &sd, sizeof( D3D11_SHADER_RESOURCE_VIEW_DESC ) );
            sd.Format              = format;
            sd.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
            sd.Texture2D.MipLevels = desc.MipLevels;

            // シェーダリソースビュー生成.
            ID3D11ShaderResourceView* pSRV;
            hr = pDevice->CreateShaderResourceView( pTexture, &sd, &pSRV );

            // エラーチェック.
            if ( FAILED( hr ) )
            {
                // テクスチャ解放.
                pTexture->Release();

                delete [] pPixels;
                pPixels = nullptr;

                // エラーログ出力.
                ELOG( "Error : ID3D11Device::CreateShaderResourceView() Failed." );

                // 異常終了.
                return false;
            }

            // シェーダリソースビューを設定.
            m_pSRV.Attach( pSRV );

            // デバッグオブジェクト名設定.
            //SetDebugObjectName( m_pSRV.GetPtr(), "asdx::Font::m_pSRV" );
        }

        delete [] pPixels;
        pPixels = nullptr;

    }

    // ファイルを閉じる.
    fclose( pFile );

    // 正常終了.
    return true;
};


//-------------------------------------------------------------------------------------------------
//          初期化処理です.
//-------------------------------------------------------------------------------------------------
bool Font::Init
(
    ID3D11Device* pDevice,
    const uint8_t*     pMemory,
    const uint32_t     binarySize
)
{
    // レンダーステートを初期化.
    if ( !RenderState::GetInstance().Init( pDevice ) )
    {
        ELOG( "Error : RenderState Initialize Failed." );
        return false;
    }

    const uint8_t* pTop = pMemory;
    uint8_t* pCur = (uint8_t*)pMemory;
    size_t offset = 0;

    // ファイルヘッダ読み込み.
    FntFileHeader* pFileHeader = (FntFileHeader*)pCur;
    offset += sizeof(FntFileHeader);
    if ( offset > binarySize )
    {
        ELOG( "Error : Out of Range. ");
        return false;
    }
    pCur += sizeof(FntFileHeader);

    // ファイルマジックをチェック.
    if ( ( pFileHeader->Magic[0] != 'F' )
      || ( pFileHeader->Magic[1] != 'N' )
      || ( pFileHeader->Magic[2] != 'T' )
      || ( pFileHeader->Magic[3] != '\0' ) )
    {
        // エラーログ出力.
        ELOGW( "Error : Invalid Binary Data. Magic = { %c, %c, %c, %c }\n",
            pFileHeader->Magic[0],
            pFileHeader->Magic[1],
            pFileHeader->Magic[2],
            pFileHeader->Magic[3] );

        // 異常終了.
        return false;
    }

    // ファイルバージョンのチェック.
    if ( pFileHeader->Version != FNT_CURRENT_FILE_VERSION )
    {
        // エラーログ出力.
        ELOG( "Error : Invalid File Version. version = 0x%x, current_version = 0x%x\n", pFileHeader->Version, FNT_CURRENT_FILE_VERSION );

        // 異常終了.
        return false;
    }

    // データヘッダ構造体サイズをチェック.
    if ( pFileHeader->DataHeaderSize != sizeof( FntDataHeader ) )
    {
        // エラーログ出力.
        ELOG( "Error : Invalid Data Header Size.  size = %d, expect_size = %d", pFileHeader->DataHeaderSize, sizeof(FntDataHeader) );

        // 異常終了.
        return false;
    }

    if ( pFileHeader->TextureHeaderSize != sizeof( FntTextureHeader ) )
    {
        // エラーログ出力.
        ELOG( "Error : Invalid Texture Header Size. size = %d, expect_size = %d", pFileHeader->TextureHeaderSize, sizeof(FntTextureHeader) );

        // 異常終了.
        return false;
    }

    // フォントサイズを設定.
    m_FontWidth  = pFileHeader->DataHeader.FontWidth;
    m_FontHeight = pFileHeader->DataHeader.FontHeight;

    // フォント名を設定.
    wcscpy_s( m_FontName, pFileHeader->DataHeader.FontName );

    // テクスチャ座標オフセットを算出.
    m_OffsetU = static_cast<float>( m_FontWidth )  / static_cast<float>( pFileHeader->TextureHeader.Width );
    m_OffsetV = static_cast<float>( m_FontHeight ) / static_cast<float>( pFileHeader->TextureHeader.Height );


    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

    // フォーマット判定.
    switch( pFileHeader->TextureHeader.Format )
    {
    case FNT_FORMAT_R8G8B8A8:
        format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;

    case FNT_FORMAT_R4G4B4A4:
        format = DXGI_FORMAT_B4G4R4A4_UNORM;
        break;

    case FNT_FORMAT_BC2:
        format = DXGI_FORMAT_BC2_UNORM;
        break;

    default:
        assert( false );
        break;
    }


    HRESULT hr = S_OK;
    {
        // テクスチャサイズ算出.
        size_t textureSize = pFileHeader->TextureHeader.Stride * pFileHeader->TextureHeader.Rows;

        uint8_t* pPixels = pCur;
        offset += sizeof(uint8_t) * textureSize;
        if ( offset > binarySize )
        {
            ELOG( "Error : Out of Range." );
            return false;
        }
        pCur   += sizeof(uint8_t) * textureSize;

        // サブリソースデータを設定.
        D3D11_SUBRESOURCE_DATA subRes;
        subRes.pSysMem          = pPixels;
        subRes.SysMemPitch      = pFileHeader->TextureHeader.Stride;
        subRes.SysMemSlicePitch = UINT( textureSize );

        // 2Dテクスチャの設定.
        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = pFileHeader->TextureHeader.Width;
        desc.Height             = pFileHeader->TextureHeader.Height;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = format;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        // テクスチャ生成.
        ID3D11Texture2D* pTexture = nullptr;
        hr = pDevice->CreateTexture2D( &desc, &subRes, &pTexture );

        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateTexture2D() Failed." );

            return false;
        }

        // テクスチャを設定.
        m_pTexture.Attach( pTexture );

        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pTexture.GetPtr(), "asdx::Font::m_pTexture" );

        {
            // シェーダリソースビューの設定.
            D3D11_SHADER_RESOURCE_VIEW_DESC sd;
            ZeroMemory( &sd, sizeof( D3D11_SHADER_RESOURCE_VIEW_DESC ) );
            sd.Format              = format;
            sd.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
            sd.Texture2D.MipLevels = desc.MipLevels;

            // シェーダリソースビュー生成.
            ID3D11ShaderResourceView* pSRV;
            hr = pDevice->CreateShaderResourceView( pTexture, &sd, &pSRV );

            // エラーチェック.
            if ( FAILED( hr ) )
            {
                // テクスチャ解放.
                pTexture->Release();

                // エラーログ出力.
                ELOG( "Error : ID3D11Device::CreateShaderResourceView() Failed." );

                // 異常終了.
                return false;
            }

            // シェーダリソースビューを設定.
            m_pSRV.Attach( pSRV );

            // デバッグオブジェクト名設定.
            //SetDebugObjectName( m_pSRV.GetPtr(), "asdx::Font::m_pSRV" );
        }
    }

    // 正常終了.
    return true;
};

//-------------------------------------------------------------------------------------------------
//          終了処理です.
//-------------------------------------------------------------------------------------------------
void Font::Term()
{
    // テクスチャを解放.
    m_pTexture.Reset();

    // シェーダリソースビューを解放.
    m_pSRV.Reset();

    memset( &m_FontName, 0, sizeof( char ) * 32 );
    m_FontName[0]   = '\0';
    m_FontWidth     = 0;
    m_FontHeight    = 0;
    m_OffsetU       = 0.0f;
    m_OffsetV       = 0.0f;
}

//-------------------------------------------------------------------------------------------------
//          フォントの横幅を設定します.
//-------------------------------------------------------------------------------------------------
void Font::SetFontWidth( uint32_t width )
{ m_FontWidth = width; }

//-------------------------------------------------------------------------------------------------
//          フォントの縦幅を設定します.
//-------------------------------------------------------------------------------------------------
void Font::SetFontHeight( uint32_t height )
{ m_FontHeight = height; }

//-------------------------------------------------------------------------------------------------
//          描画開始処理です.
//-------------------------------------------------------------------------------------------------
void Font::Begin( ID3D11DeviceContext* pDeviceContext, Sprite& sprite )
{
    // ブレンドステートを設定します.
    pDeviceContext->OMSetBlendState( RenderState::GetInstance().GetBS( BlendType::AlphaBlend ), nullptr, 0xFFFFFFFF );

    // 深度ステンシルステートを設定します.
    pDeviceContext->OMSetDepthStencilState( RenderState::GetInstance().GetDSS( DepthType::Default ), 0 );

    // ラスタライザーステートを設定します.
    pDeviceContext->RSSetState( RenderState::GetInstance().GetRS( RasterizerType::CullNone ) );

    sprite.SetTexture( m_pSRV.GetPtr(), RenderState::GetInstance().GetSmp( SamplerType::LinearClamp ) );
    sprite.Begin( pDeviceContext, Sprite::SHADER_TYPE_TEXTURE2D );
}

//-------------------------------------------------------------------------------------------------
//          文字列を描画します.
//-------------------------------------------------------------------------------------------------
void Font::DrawString( Sprite& sprite, const int x, const int y, const char* text )
{ DrawString( sprite, x, y, 0, text ); }

//-------------------------------------------------------------------------------------------------
//          文字列を描画します.
//-------------------------------------------------------------------------------------------------
void Font::DrawString( Sprite& sprite, const int x, const int y, const int layerDepth, const char* text )
{
    uint32_t stringsCount = (uint32_t)strlen( text );

    // 文字列がなければ何もしない.
    if ( stringsCount == 0 )
    { return; }

    // 開始位置を設定.
    auto posX = x;
    auto posY = y;

    // 文字数分ループ.
    for( uint32_t i=0; i<stringsCount; i++ )
    {
        if ( (uint32_t)text[i] == 0x0a )
        {
            posY += ( m_FontHeight + 1 );
            posX  = x;
        }
        // インデックス算出.
        uint32_t idx = (uint32_t)text[i] - 0x20;

        if ( idx > 95 )
        { continue; }

        // テクスチャ座標.
        Vector2 uv0( ( idx + 0 ) * m_OffsetU, 0.0f );
        Vector2 uv1( ( idx + 1 ) * m_OffsetU, m_OffsetV );

        // 文字データ追加.
        sprite.Draw( posX, posY, m_FontWidth, m_FontHeight, uv0, uv1, layerDepth );

        // 横方向に移動.
        posX += m_FontWidth;
    }
}

//-------------------------------------------------------------------------------------------------
//          書式指定して文字列を描画します.
//-------------------------------------------------------------------------------------------------
void Font::DrawStringArg( Sprite& sprite, const int x, const int y, const char* format, ... )
{
    int result = 0;
    va_list arg;

    va_start( arg, format );
    result = vsprintf_s( m_Buffer, format, arg );
    va_end( arg );

    DrawString( sprite, x, y, 0, m_Buffer );
}

//-------------------------------------------------------------------------------------------------
//          書式指定して文字列を描画します.
//-------------------------------------------------------------------------------------------------
void Font::DrawStringArg( Sprite& sprite, const int x, const int y, const int layerDepth, const char* format, ... )
{
    int result = 0;
    va_list arg;

    va_start( arg, format );
    result = vsprintf_s( m_Buffer, format, arg );
    va_end( arg );

    DrawString( sprite, x, y, layerDepth, m_Buffer );
}

//-------------------------------------------------------------------------------------------------
//          描画終了です.
//-------------------------------------------------------------------------------------------------
void Font::End( ID3D11DeviceContext* pDeviceContext, Sprite& sprite )
{
    sprite.End( pDeviceContext );

    // ステートをデフォルトに設定.
    pDeviceContext->OMSetBlendState( RenderState::GetInstance().GetBS( BlendType::Opaque ), nullptr, 0xFFFFFFFF );
    pDeviceContext->OMSetDepthStencilState( RenderState::GetInstance().GetDSS( DepthType::Default ), 0 );
    pDeviceContext->RSSetState( RenderState::GetInstance().GetRS( RasterizerType::CullCounterClockWise ) );
}

//-------------------------------------------------------------------------------------------------
//          フォントの横幅を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Font::GetFontWidth() const
{ return m_FontWidth; }

//-------------------------------------------------------------------------------------------------
//          フォントの縦幅を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Font::GetFontHeight() const
{ return m_FontHeight; }

//-------------------------------------------------------------------------------------------------
//          フォント名を取得します.
//-------------------------------------------------------------------------------------------------
const wchar_t* Font::GetFontName() const
{ return m_FontName; }


} // namespace asdx
