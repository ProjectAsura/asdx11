//-------------------------------------------------------------------------------------------------
// File : asdxTexture.cpp
// Desc : Texture Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTexture.h>
#include <asdxMisc.h>
#include <cassert>


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//! @brief      2次元テクスチャを生成します.
//!
//! @param [in]     pDevice         デバイスです.
//! @param [in]     width           テクスチャの横幅です.
//! @param [in]     height          テクスチャの縦幅です.
//! @param [in]     mipCount        ミップマップ数です.
//! @param [in]     surfaceCount    サーフェイス数です.
//! @param [in]     isCubeMap       キューブマップかどうか.
//! @param [in]     format          DXGIフォーマットです.
//! @param [in]     usage           使用方法.
//! @param [in]     bindFlags       バインドフラグです.
//! @param [in]     cpuAccessFlags  CPUAアクセスフラグです.
//! @param [in]     miscFlags       その他のオプション.
//! @param [in]     pInitData       サブリソースです.
//! @param [out]    ppTexture       テクスチャです.
//! @param [out]    ppSRV           シェーダリソースビューです.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
bool CreateTexture2D
(
    ID3D11Device*               pDevice,
    ID3D11DeviceContext*        pDeviceContext,
    UINT                        width,
    UINT                        height,
    UINT                        mipCount,
    UINT                        surfaceCount,
    bool                        isCubeMap,
    DXGI_FORMAT                 format,
    D3D11_USAGE                 usage,
    UINT                        bindFlags,
    UINT                        cpuAccessFlags,
    UINT                        miscFlags,
    D3D11_SUBRESOURCE_DATA*     pInitData,
    ID3D11Texture2D**           ppTexture,
    ID3D11ShaderResourceView**  ppSRV
)
{
    assert( pInitData != nullptr );
    if ( mipCount == 0 )
    { mipCount = 1; }

    bool autogen = false;
    uint32_t fmtSupport = 0;
    HRESULT hr = pDevice->CheckFormatSupport( format, &fmtSupport );
    if ( mipCount == 1 && SUCCEEDED( hr ) && ( fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN ) )
    {
        autogen = true;
    }

    // テクスチャ2Dの設定
    D3D11_TEXTURE2D_DESC td;

    td.Width              = width;
    td.Height             = height;
    td.MipLevels          = ( autogen ) ? 0 : mipCount;
    td.ArraySize          = surfaceCount;
    td.Format             = format;
    td.SampleDesc.Count   = 1;
    td.SampleDesc.Quality = 0;
    td.Usage              = usage;
    td.BindFlags          = bindFlags;
    td.CPUAccessFlags     = cpuAccessFlags;
    if ( isCubeMap )
    { td.MiscFlags = miscFlags | D3D11_RESOURCE_MISC_TEXTURECUBE; }
    else
    { td.MiscFlags = miscFlags & ~D3D11_RESOURCE_MISC_TEXTURECUBE; }

    if ( autogen )
    {
        td.BindFlags |= D3D11_BIND_RENDER_TARGET;
        td.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    // テクスチャを生成.
    ID3D11Texture2D* pTexture = nullptr;
    if ( autogen )
    {
        hr = pDevice->CreateTexture2D( &td, nullptr, &pTexture );
    }
    else
    {
        hr = pDevice->CreateTexture2D( &td, pInitData, &pTexture );
    }

    // チェック.
    if ( SUCCEEDED( hr ) && ( pTexture != nullptr ) )
    {
        //asdx::SetDebugObjectName( pTexture, "asdx::Texture2D" );

        // シェーダリソースビューが指定されている場合.
        if ( ppSRV != nullptr )
        {
            // シェーダリソースビューの設定.
            D3D11_SHADER_RESOURCE_VIEW_DESC sd;

            // ゼロクリア.
            memset( &sd, 0, sizeof( sd ) );

            // 画像フォーマットの設定.
            sd.Format = format;

            // キューブマップの場合.
            if ( isCubeMap )
            {
                sd.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
                sd.TextureCube.MipLevels = ( autogen ) ? -1 : mipCount;
            }
            // 普通の2Dテクスチャの場合.
            else
            {
                sd.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
                sd.TextureCube.MipLevels = ( autogen ) ? -1 : mipCount;
            }

            // シェーダリソースビューを生成.
            hr = pDevice->CreateShaderResourceView( pTexture, &sd, ppSRV );

            // チェック.
            if ( FAILED( hr ) )
            {
                // テクスチャを解放.
                pTexture->Release();

                // 異常終了.
                return false;
            }

            //asdx::SetDebugObjectName( (*ppSRV), "asdx::Texture2D" );

            if ( autogen )
            {
                if ( surfaceCount <= 1 )
                {
                    pDeviceContext->UpdateSubresource( pTexture, 0, nullptr, pInitData->pSysMem, pInitData->SysMemPitch, pInitData->SysMemSlicePitch );
                }
                else
                {
                    D3D11_TEXTURE2D_DESC desc;
                    pTexture->GetDesc( &desc );

                    for( uint32_t i=0; i<surfaceCount; ++i )
                    {
                        auto res = D3D11CalcSubresource( 0, i, desc.MipLevels );
                        pDeviceContext->UpdateSubresource( pTexture, res, nullptr, pInitData[i].pSysMem, pInitData[i].SysMemPitch, pInitData[i].SysMemSlicePitch );
                    }
                }
                pDeviceContext->GenerateMips( (*ppSRV) );
            }
        }

        // テクスチャが指定されている場合.
        if ( ppTexture != nullptr )
        {
            // テクスチャを設定.
            (*ppTexture) = pTexture;
        }
        else
        {
            // テクスチャを解放.
            pTexture->Release();
        }

        // 正常終了.
        return true;
    }


    // 異常終了.
    return false;
}


//-------------------------------------------------------------------------------------------------
//! @brief      3次元テクスチャを生成します.
//!
//! @param [in]     pDevice         デバイスです.
//! @param [in]     width           テクスチャの横幅です.
//! @param [in]     height          テクスチャの縦幅です.
//! @param [in]     depth           テクスチャの奥行です.
//! @param [in]     mipCount        ミップマップ数です.
//! @param [in]     format          DXGIフォーマットです.
//! @param [in]     usage           使用方法.
//! @param [in]     bindFlags       バインドフラグです.
//! @param [in]     cpuAccessFlags  CPUAアクセスフラグです.
//! @param [in]     miscFlags       その他のオプション.
//! @param [in]     pInitData       サブリソースです.
//! @param [out]    ppTexture       テクスチャです.
//! @param [out]    ppSRV           シェーダリソースビューです.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
bool CreateTexture3D
(
    ID3D11Device*               pDevice,
    UINT                        width,
    UINT                        height,
    UINT                        depth,
    UINT                        mipCount,
    DXGI_FORMAT                 format,
    D3D11_USAGE                 usage,
    UINT                        bindFlags,
    UINT                        cpuAccessFlags,
    UINT                        miscFlags,
    D3D11_SUBRESOURCE_DATA*     pInitData,
    ID3D11Texture3D**           ppTexture,
    ID3D11ShaderResourceView**  ppSRV
)
{
    assert( pInitData != nullptr );
    HRESULT hr = S_OK;

    // テクスチャ3Dの設定.
    D3D11_TEXTURE3D_DESC td;

    td.Width          = width;
    td.Height         = height;
    td.Depth          = depth;
    td.MipLevels      = mipCount;
    td.Format         = format;
    td.Usage          = usage;
    td.BindFlags      = bindFlags;
    td.CPUAccessFlags = cpuAccessFlags;
    td.MiscFlags      = miscFlags & ~D3D11_RESOURCE_MISC_TEXTURECUBE;


    // テクスチャを生成.
    ID3D11Texture3D* pTexture = nullptr;
    hr = pDevice->CreateTexture3D( &td, pInitData, &pTexture );

    // チェック.
    if ( SUCCEEDED( hr ) && pTexture != nullptr )
    {
        // シェーダリソースビューが指定されている場合.
        if ( ppSRV != nullptr )
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC sd;

            // ゼロクリア.
            memset( &sd, 0, sizeof(sd) );

            // シェーダリソースビューの設定.
            sd.Format = format;
            sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
            sd.Texture3D.MipLevels = td.MipLevels;

            // シェーダリソースビューを生成.
            hr = pDevice->CreateShaderResourceView( pTexture, &sd, ppSRV );

            // エラーチェック.
            if ( FAILED( hr ) )
            {
                // 解放.
                pTexture->Release();

                // 異常終了.
                return false;
            }
        }

        // テクスチャが指定されている場合.
        if ( pTexture != nullptr )
        {
            // テクスチャを設定.
            (*ppTexture) = pTexture;
        }
        else
        {
            // 解放.
            pTexture->Release();
        }

        // 正常終了.
        return true;
    }

    // 異常終了.
    return false;
}


} // namespace /* anonymous */


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Texture2D class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Texture2D::Texture2D()
: m_Format  ( DXGI_FORMAT_UNKNOWN )
, m_pTexture( nullptr )
, m_pSRV    ( nullptr )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      コピーコンストラクタです.
//-------------------------------------------------------------------------------------------------
Texture2D::Texture2D( const Texture2D& value )
: m_Format  ( value.m_Format )
, m_pTexture( value.m_pTexture )
, m_pSRV    ( value.m_pSRV )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Texture2D::~Texture2D()
{ Release(); }

//-------------------------------------------------------------------------------------------------
//      メモリ解放処理です.
//-------------------------------------------------------------------------------------------------
void Texture2D::Release()
{
    m_pTexture.Reset();
    m_pSRV.Reset();

    m_Format = DXGI_FORMAT_UNKNOWN;
}

//-------------------------------------------------------------------------------------------------
//      ファイルからテクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool Texture2D::Create( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const ResTexture& resource )
{
    // フォーマットを設定.
    m_Format = static_cast<DXGI_FORMAT>( resource.Format );

    // メモリを確保.
    D3D11_SUBRESOURCE_DATA* pSubRes = new D3D11_SUBRESOURCE_DATA[ resource.MipMapCount * resource.SurfaceCount ];

    // NULLチェック.
    if ( pSubRes == nullptr )
    {
        // 異常終了.
        return false;
    }

    // サブリソースを設定.
    for( uint32_t j=0; j<resource.SurfaceCount; ++j )
    {
        for( uint32_t i=0; i<resource.MipMapCount; ++i )
        {
            int idx = ( resource.MipMapCount * j ) + i;
            pSubRes[ idx ].pSysMem          = ( const void* )resource.pResources[ idx ].pPixels;
            pSubRes[ idx ].SysMemPitch      = resource.pResources[ idx ].Pitch;
            pSubRes[ idx ].SysMemSlicePitch = resource.pResources[ idx ].SlicePitch;
        }
    }

    ID3D11Texture2D* pTexture;
    ID3D11ShaderResourceView* pSRV;

    // テクスチャを生成.
    bool result = CreateTexture2D(
                        pDevice,
                        pDeviceContext,
                        resource.Width,
                        resource.Height,
                        resource.MipMapCount,
                        resource.SurfaceCount,
                        (( resource.Option & SUBRESOURCE_OPTION_CUBEMAP ) > 0),
                        m_Format,
                        D3D11_USAGE_DEFAULT,
                        D3D11_BIND_SHADER_RESOURCE,
                        0,
                        0,
                        pSubRes,
                        &pTexture,
                        &pSRV );

    delete [] pSubRes;
    pSubRes = nullptr;

    if ( result )
    {
        m_pTexture.Attach( pTexture );
        m_pSRV.Attach( pSRV );
    }

    // 処理結果を返却.
    return result;
}

//-------------------------------------------------------------------------------------------------
//      フォーマットを取得します.
//-------------------------------------------------------------------------------------------------
int Texture2D::GetFormat() const
{ return m_Format; }

//-------------------------------------------------------------------------------------------------
//      テクスチャを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Texture2D* const Texture2D::GetTexture() const
{ return m_pTexture.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* const Texture2D::GetSRV() const
{ return m_pSRV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      代入演算子です.
//-------------------------------------------------------------------------------------------------
Texture2D& Texture2D::operator = ( const Texture2D& value )
{
    m_Format   = value.m_Format;
    m_pTexture = value.m_pTexture;
    m_pSRV     = value.m_pSRV;

    return (*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Texture3D class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Texture3D::Texture3D()
: m_Format  ( DXGI_FORMAT_UNKNOWN )
, m_pTexture( nullptr )
, m_pSRV    ( nullptr )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Texture3D::~Texture3D()
{ Release(); }

//-------------------------------------------------------------------------------------------------
//      メモリ解放処理です.
//-------------------------------------------------------------------------------------------------
void Texture3D::Release()
{
    m_pTexture.Reset();
    m_pSRV.Reset();

    m_Format = DXGI_FORMAT_UNKNOWN;
}

//-------------------------------------------------------------------------------------------------
//      ファイルからテクスチャを生成します.
//-------------------------------------------------------------------------------------------------
bool Texture3D::Create( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const ResTexture& resource )
{
    // NULLチェック.
    if ( pDevice == nullptr )
    {
        // エラーログ出力.
        printf_s( "Error : Invalid Argument.\n" );

        // 異常終了.
        return false;
    }

      // フォーマットを設定.
    m_Format = static_cast<DXGI_FORMAT>( resource.Format );

    // メモリを確保.
    D3D11_SUBRESOURCE_DATA* pSubRes = new D3D11_SUBRESOURCE_DATA[ resource.MipMapCount * resource.SurfaceCount ];

    // NULLチェック.
    if ( pSubRes == nullptr )
    {
        // 異常終了.
        return false;
    }

    // サブリソースを設定.
    for( uint32_t j=0; j<resource.SurfaceCount; ++j )
    {
        for( uint32_t i=0; i<resource.MipMapCount; ++i )
        {
            int idx = ( resource.MipMapCount * j ) + i;
            pSubRes[ idx ].pSysMem          = ( const void* )resource.pResources[ idx ].pPixels;
            pSubRes[ idx ].SysMemPitch      = resource.pResources[ idx ].Pitch;
            pSubRes[ idx ].SysMemSlicePitch = resource.pResources[ idx ].SlicePitch;
        }
    }

    ID3D11Texture3D* pTexture;
    ID3D11ShaderResourceView* pSRV;

    // テクスチャを生成.
    bool result = CreateTexture3D(
                        pDevice,
                        resource.Width,
                        resource.Height,
                        resource.Depth,
                        resource.MipMapCount,
                        m_Format,
                        D3D11_USAGE_DEFAULT,
                        D3D11_BIND_SHADER_RESOURCE,
                        0,
                        0,
                        pSubRes,
                        &pTexture,
                        &pSRV );

    // 不要になったメモリを解放
    delete [] pSubRes;
    pSubRes = nullptr;

    if ( result )
    {
        m_pTexture.Attach( pTexture );
        m_pSRV.Attach( pSRV );
    }

    // 処理結果を返却.
    return result;
}

//-------------------------------------------------------------------------------------------------
//      テクスチャを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Texture3D* const Texture3D::GetTexture() const
{ return m_pTexture.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* const Texture3D::GetSRV() const
{ return m_pSRV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      代入演算子です.
//-------------------------------------------------------------------------------------------------
Texture3D& Texture3D::operator = ( const Texture3D& value )
{
    m_Format   = value.m_Format;
    m_pTexture = value.m_pTexture;
    m_pSRV     = value.m_pSRV;

    return (*this);
}

} // namespacea asdx
