//-------------------------------------------------------------------------------------------------
// File : dfw_RenderTarget2D.cpp
// Desc : RenderTarget2D Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cassert>
#include <asdxTarget.h>
#include <asdxMisc.h>
#include <asdxLogger.h>


namespace /* anonymous */ {

 //------------------------------------------------------------------------------------------------
 //! @brief     適切なフォーマットに変換します.
 //!
 //! @param [in]        format              入力フォーマット.
 //! @param [out]       textureFormat       テクスチャフォーマット.
 //! @param [out]       resourceFormat      リソースフォーマット.
 //------------------------------------------------------------------------------------------------
void ConvertDepthFormat
(
    const DXGI_FORMAT& format,
    DXGI_FORMAT&       textureFormat,
    DXGI_FORMAT&       resourceFormat
)
{
    // 入力フォーマットで初期化.
    textureFormat  = format;
    resourceFormat = format;

    // 適切なフォーマットを取得.
    switch( format )
    {
    case DXGI_FORMAT_D16_UNORM:
        {
            textureFormat  = DXGI_FORMAT_R16_TYPELESS;
            resourceFormat = DXGI_FORMAT_R16_UNORM;
        }
        break;

    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        {
            textureFormat  = DXGI_FORMAT_R24G8_TYPELESS;
            resourceFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        }
        break;

    case DXGI_FORMAT_D32_FLOAT:
        {
            textureFormat  = DXGI_FORMAT_R32_TYPELESS;
            resourceFormat = DXGI_FORMAT_R32_FLOAT;
        }
        break;

    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        {
            textureFormat  = DXGI_FORMAT_R32G8X24_TYPELESS;
            resourceFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        }
        break;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      適切なフォーマットに変換します.
//!
//! @param [in]     format              入力フォーマット.
//! @param [out]    textureFormat       テクスチャフォーマット.
//-------------------------------------------------------------------------------------------------
void ConvertColorFormat
(
    const DXGI_FORMAT&  format,
    DXGI_FORMAT&        textureFormat
)
{
    textureFormat  = format;

    switch( format )
    {
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        {
            textureFormat = DXGI_FORMAT_R32G32B32A32_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        {
            textureFormat = DXGI_FORMAT_R32G32B32_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
        {
            textureFormat = DXGI_FORMAT_R16G16B16A16_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
        {
            textureFormat = DXGI_FORMAT_R32G32_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        {
            textureFormat = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        }
        break;

    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
        {
            textureFormat = DXGI_FORMAT_R10G10B10A2_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
        {
            textureFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
        {
            textureFormat = DXGI_FORMAT_R16G16_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
        {
            textureFormat = DXGI_FORMAT_R32_TYPELESS;
        }
        break;

    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        {
            textureFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
        {
            textureFormat = DXGI_FORMAT_R8G8_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
        {
            textureFormat = DXGI_FORMAT_R16_TYPELESS;
        }
        break;

    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
        {
            textureFormat = DXGI_FORMAT_R8_TYPELESS;
        }
        break;

    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
        {
            textureFormat = DXGI_FORMAT_BC1_TYPELESS;
        }
        break;

    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
        {
            textureFormat = DXGI_FORMAT_BC2_TYPELESS;
        }
        break;

    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
        {
            textureFormat = DXGI_FORMAT_BC3_TYPELESS;
        }
        break;

    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        {
            textureFormat = DXGI_FORMAT_BC4_TYPELESS;
        }
        break;

    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
        {
            textureFormat = DXGI_FORMAT_BC5_TYPELESS;
        }
        break;

    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        {
            textureFormat = DXGI_FORMAT_B8G8R8A8_TYPELESS;
        }
        break;

    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        {
            textureFormat = DXGI_FORMAT_B8G8R8X8_TYPELESS;
        }
        break;

    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
        {
            textureFormat = DXGI_FORMAT_BC6H_TYPELESS;
        }
        break;

    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        {
            textureFormat = DXGI_FORMAT_BC7_TYPELESS;
        }
        break;
    }
}

} // namespace /* anoymous */

namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ColorTarget2D class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ColorTarget2D::ColorTarget2D()
: m_pTexture( nullptr )
, m_pSRV    ( nullptr )
, m_pRTV    ( nullptr )
, m_Desc    ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ColorTarget2D::~ColorTarget2D()
{ Release(); }

//-------------------------------------------------------------------------------------------------
//      バックバッファから描画ターゲットを生成します.
//-------------------------------------------------------------------------------------------------
bool ColorTarget2D::CreateFromBackBuffer( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain )
{
    HRESULT hr = S_OK;

    // バックバッファを取得.
    hr = pSwapChain->GetBuffer( 0, IID_PPV_ARGS(m_pTexture.GetAddress()) );
    if( FAILED( hr ) )
    {
        // エラーログを出力.
        ELOG( "Error : IDXGISwapChain->GetBuffer() Failed. call from ColorTarget::CreateFromBackBuffer()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pTexture.GetPtr(), "ColorTarget2D::m_pTexture" );

    // レンダーターゲットビューを生成.
    hr = pDevice->CreateRenderTargetView( m_pTexture.GetPtr(), nullptr, m_pRTV.GetAddress() );
    if ( FAILED( hr ) )
    {
        // バックバッファを解放.
        m_pTexture.Reset();

        // エラーログを出力.
        ELOG( "Error : ID3D11Device::CreateRenderTargetView() Failed. call from ColorTarget::CreateFromBackBuffer()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pRTV.GetPtr(), "ColorTarget2D::m_pRTV" );

    // シェーダリソースビューを生成.
    hr = pDevice->CreateShaderResourceView( m_pTexture.GetPtr(), nullptr, m_pSRV.GetAddress() );
    if ( FAILED( hr ) )
    {
        // テクスチャを解放.
        m_pTexture.Reset();

        // レンダーターゲットビューを解放.
        m_pRTV.Reset();

        // エラーログを出力.
        ELOG( "Error : ID3D11Device::CreateShaderResourcdeView() Failed. call from ColorTarget::CreateFromBackBuffer()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pSRV.GetPtr(), "ColorTarget2D::m_pSRV" );

    D3D11_TEXTURE2D_DESC desc;
    m_pTexture->GetDesc( &desc );
    m_Desc.Width          = desc.Width;
    m_Desc.Height         = desc.Height;
    m_Desc.ArraySize      = desc.ArraySize;
    m_Desc.CPUAccessFlags = desc.CPUAccessFlags;
    m_Desc.Format         = desc.Format;
    m_Desc.MipLevels      = desc.MipLevels;
    m_Desc.MiscFlags      = desc.MiscFlags;
    m_Desc.SampleDesc     = desc.SampleDesc;

    // 成功.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      描画ターゲットを生成します.
//-------------------------------------------------------------------------------------------------
bool ColorTarget2D::Create( ID3D11Device* pDevice, const TargetDesc2D& desc )
{
    assert( desc.ArraySize >= 1 );
    HRESULT hr = S_OK;

    DXGI_FORMAT textureFormat;

    // 適切なフォーマットに変換.
    ConvertColorFormat( desc.Format, textureFormat );

    // テクスチャの設定.
    D3D11_TEXTURE2D_DESC td = {};
    td.Width                = desc.Width;
    td.Height               = desc.Height;
    td.MipLevels            = desc.MipLevels;
    td.ArraySize            = desc.ArraySize;
    td.Format               = textureFormat;
    td.SampleDesc.Count     = desc.SampleDesc.Count;
    td.SampleDesc.Quality   = desc.SampleDesc.Quality;
    td.Usage                = D3D11_USAGE_DEFAULT;
    td.BindFlags            = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags       = desc.CPUAccessFlags;
    td.MiscFlags            = desc.MiscFlags;

    // テクスチャを生成します.
    hr = pDevice->CreateTexture2D( &td, nullptr, m_pTexture.GetAddress() );
    if ( FAILED( hr ) )
    {
        // エラーログ出力.
        ELOG( "Error : ID3D11Device::CreateTexture2D() Failed. call from ColorTarget2D::Create(), errcode = 0x%x", hr );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pTexture.GetPtr(), "ColorTarget2D::m_pTexture" );

    // レンダーターゲットビューの設定.
    D3D11_RENDER_TARGET_VIEW_DESC rtvd = {};
    rtvd.Format = desc.Format;

    // 配列サイズが1の場合.
    if ( desc.ArraySize == 1 )
    {
        // マルチサンプリング無しの場合.
        if ( desc.SampleDesc.Count <= 1 )
        {
            rtvd.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvd.Texture2D.MipSlice = 0;
        }
        // マルチサンプリング有りの場合.
        else
        {
            rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        }
    }
    // 配列サイズが1より大きい場合.
    else
    {
        // マルチサンプリング無しの場合.
        if ( desc.SampleDesc.Count <= 1 )
        {
            rtvd.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvd.Texture2DArray.ArraySize       = desc.ArraySize;
            rtvd.Texture2DArray.FirstArraySlice = 0;
            rtvd.Texture2DArray.MipSlice        = 0;
        }
        // マルチサンプリング有りの場合.
        else
        {
            rtvd.ViewDimension                    = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
            rtvd.Texture2DMSArray.ArraySize       = desc.ArraySize;
            rtvd.Texture2DMSArray.FirstArraySlice = 0;
        }
    }

    // レンダーターゲットビューを生成.
    hr = pDevice->CreateRenderTargetView( m_pTexture.GetPtr(), &rtvd, m_pRTV.GetAddress() );
    if ( FAILED( hr ) )
    {
        // テクスチャを解放.
        m_pTexture.Reset();

        // エラーログ出力.
        ELOG( "Error : ID3D11Device::CreateRenderTargetView() Failed. call from ColorTarget::Create()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pRTV.GetPtr(), "ColorTarget2D::m_pRTV" );

    // シェーダリソースビューの設定.
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = desc.Format;

    // 配列サイズが1の場合.
    if ( desc.ArraySize == 1 )
    {
        // マルチサンプリング無しの場合.
        if ( desc.SampleDesc.Count <= 1 )
        {
            srvd.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvd.Texture2D.MipLevels       = desc.MipLevels;
            srvd.Texture2D.MostDetailedMip = 0;
        }
        // マルチサンプリング有りの場合.
        else
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        }
    }
    // 配列サイズが1より大きい場合.
    else
    {
        // マルチサンプリング無しの場合.
        if ( desc.SampleDesc.Count <= 1 )
        {
            srvd.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            srvd.Texture2DArray.ArraySize       = desc.ArraySize;
            srvd.Texture2DArray.FirstArraySlice = 0;
            srvd.Texture2DArray.MipLevels       = desc.MipLevels;
            srvd.Texture2DArray.MostDetailedMip = 0;
        }
        // マルチサンプリング有りの場合.
        else
        {
            srvd.ViewDimension                    = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
            srvd.Texture2DMSArray.ArraySize       = desc.ArraySize;
            srvd.Texture2DMSArray.FirstArraySlice = 0;
        }
    }

    // シェーダリソースビューを生成.
    hr = pDevice->CreateShaderResourceView( m_pTexture.GetPtr(), &srvd, m_pSRV.GetAddress() );
    if ( FAILED( hr ) )
    {
        // テクスチャを解放.
        m_pTexture.Reset();

        // レンダーターゲットビューを解放.
        m_pRTV.Reset();

        // エラーログを出力.
        ELOG( "Error : ID3D11Device::CreateShaderResourceView() Failed. call from ColorTarget::Create()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pSRV.GetPtr(), "ColorTarget2D::m_pSRV" );

    m_Desc = desc;

    // 成功.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      リサイズ処理を行います.
//-------------------------------------------------------------------------------------------------
bool ColorTarget2D::Resize(ID3D11Device* pDevice, uint32_t width, uint32_t height)
{
    // サイズが同じ場合は変更せずに成功扱いにする.
    if (m_Desc.Width == width && m_Desc.Height == height)
    { return true; }

    m_pRTV.Reset();
    m_pSRV.Reset();
    m_pTexture.Reset();

    m_Desc.Width  = width;
    m_Desc.Height = height;

    return Create(pDevice, m_Desc);
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetDesc2D ColorTarget2D::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      解放処理です.
//-------------------------------------------------------------------------------------------------
void ColorTarget2D::Release()
{
    m_pRTV.Reset();
    m_pSRV.Reset();
    m_pTexture.Reset();

    m_Desc = TargetDesc2D();
}

//-------------------------------------------------------------------------------------------------
//      テクスチャを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Texture2D* const ColorTarget2D::GetResource() const
{ return m_pTexture.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* const ColorTarget2D::GetShaderResource() const
{ return m_pSRV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11RenderTargetView* const ColorTarget2D::GetTargetView() const
{ return m_pRTV.GetPtr(); }


///////////////////////////////////////////////////////////////////////////////////////////////////
// ColorTarget3D class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ColorTarget3D::ColorTarget3D()
: m_pTexture( nullptr )
, m_pSRV    ( nullptr )
, m_pRTV    ( nullptr )
, m_Desc    ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ColorTarget3D::~ColorTarget3D()
{ Release(); }

//-------------------------------------------------------------------------------------------------
//      描画ターゲットを生成します.
//-------------------------------------------------------------------------------------------------
bool ColorTarget3D::Create( ID3D11Device* pDevice, const TargetDesc3D& desc )
{
    HRESULT hr = S_OK;

    DXGI_FORMAT textureFormat;

    // 適切なフォーマットに変換.
    ConvertColorFormat( desc.Format, textureFormat );

    // テクスチャの設定.
    D3D11_TEXTURE3D_DESC td = {};
    td.Width                = desc.Width;
    td.Height               = desc.Height;
    td.Depth                = desc.Depth;
    td.MipLevels            = desc.MipLevels;
    td.Format               = textureFormat;
    td.Usage                = D3D11_USAGE_DEFAULT;
    td.BindFlags            = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags       = desc.CPUAccessFlags;
    td.MiscFlags            = desc.MiscFlags;

    // テクスチャを生成します.
    hr = pDevice->CreateTexture3D( &td, nullptr, m_pTexture.GetAddress() );
    if ( FAILED( hr ) )
    {
        // エラーログ出力.
        ELOG( "Error : ID3D11Device::CreateTexture2D() Failed. call from ColorTarget::Create()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pTexture.GetPtr(), "ColorTarget3D::m_pTexture" );

    // レンダーターゲットビューの設定.
    D3D11_RENDER_TARGET_VIEW_DESC rtvd;
    ZeroMemory( &rtvd, sizeof( D3D11_RENDER_TARGET_VIEW_DESC ) );
    rtvd.Format                = desc.Format;
    rtvd.ViewDimension         = D3D11_RTV_DIMENSION_TEXTURE3D;
    rtvd.Texture3D.FirstWSlice = 0;
    rtvd.Texture3D.MipSlice    = 0;
    rtvd.Texture3D.WSize       = desc.Depth;

    // レンダーターゲットビューを生成.
    hr = pDevice->CreateRenderTargetView( m_pTexture.GetPtr(), &rtvd, m_pRTV.GetAddress() );
    if ( FAILED( hr ) )
    {
        // テクスチャを解放.
        m_pTexture.Reset();

        // エラーログ出力.
        ELOG( "Error : ID3D11Device::CreateRenderTargetView() Failed. call from ColorTarget::Create()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pRTV.GetPtr(), "ColorTarget3D::m_pRTV" );

    // シェーダリソースビューの設定.
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format                     = desc.Format;
    srvd.Texture3D.MipLevels        = desc.MipLevels;
    srvd.Texture3D.MostDetailedMip  = 0;

    // シェーダリソースビューを生成.
    hr = pDevice->CreateShaderResourceView( m_pTexture.GetPtr(), &srvd, m_pSRV.GetAddress() );
    if ( FAILED( hr ) )
    {
        // テクスチャを解放.
        m_pTexture.Reset();

        // レンダーターゲットビューを解放.
        m_pRTV.Reset();

        // エラーログを出力.
        ELOG( "Error : ID3D11Device::CreateShaderResourceView() Failed. call from ColorTarget::Create()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pSRV.GetPtr(), "ColorTarget::m_pSRV" );

    m_Desc = desc;

    // 成功.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      リサイズ処理を行います.
//-------------------------------------------------------------------------------------------------
bool ColorTarget3D::Resize(ID3D11Device* pDevice, uint32_t width, uint32_t height, uint32_t depth)
{
    // サイズが同じ場合は変更せずに成功扱いにする.
    if (m_Desc.Width == width && m_Desc.Height == height && m_Desc.Depth == depth)
    { return true; }

    m_pSRV.Reset();
    m_pRTV.Reset();
    m_pTexture.Reset();

    m_Desc.Width  = width;
    m_Desc.Height = height;
    m_Desc.Depth  = depth;

    return Create(pDevice, m_Desc);
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetDesc3D ColorTarget3D::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      解放処理です.
//-------------------------------------------------------------------------------------------------
void ColorTarget3D::Release()
{
    m_pSRV.Reset();
    m_pRTV.Reset();
    m_pTexture.Reset();

    m_Desc = TargetDesc3D();
}

//-------------------------------------------------------------------------------------------------
//      テクスチャを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Texture3D* const ColorTarget3D::GetResource() const
{ return m_pTexture.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* const ColorTarget3D::GetShaderResource() const
{ return m_pSRV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11RenderTargetView* const ColorTarget3D::GetTargetView() const
{ return m_pRTV.GetPtr(); }



///////////////////////////////////////////////////////////////////////////////////////////////////
// DepthTarget2D class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DepthTarget2D::DepthTarget2D()
: m_pTexture( nullptr )
, m_pSRV    ( nullptr )
, m_pDSV    ( nullptr )
, m_Desc    ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DepthTarget2D::~DepthTarget2D()
{ Release(); }

//-------------------------------------------------------------------------------------------------
//      深度ステンシルバッファを生成します.
//-------------------------------------------------------------------------------------------------
bool DepthTarget2D::Create( ID3D11Device* pDevice, const TargetDesc2D& desc )
{
    assert( desc.ArraySize >= 1 );
    HRESULT hr = S_OK;

    DXGI_FORMAT textureFormat;
    DXGI_FORMAT resourceFormat;

    // 適切なフォーマットに変換.
    ConvertDepthFormat( desc.Format, textureFormat, resourceFormat );

    // テクスチャの設定.
    D3D11_TEXTURE2D_DESC td = {};
    td.Width                = desc.Width;
    td.Height               = desc.Height;
    td.MipLevels            = desc.MipLevels;
    td.ArraySize            = desc.ArraySize;
    td.Format               = textureFormat;
    td.SampleDesc.Count     = desc.SampleDesc.Count;
    td.SampleDesc.Quality   = desc.SampleDesc.Quality;
    td.Usage                = D3D11_USAGE_DEFAULT;
    td.BindFlags            = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags       = desc.CPUAccessFlags;
    td.MiscFlags            = desc.MiscFlags;

    // テクスチャを生成.
    hr = pDevice->CreateTexture2D( &td, nullptr, m_pTexture.GetAddress() );
    if ( FAILED( hr ) )
    {
        // エラーログを出力.
        ELOG( "Error : ID3D11Device::CreateTexture2D() Failed. call from DepthTarget::Create()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pTexture.GetPtr(), "DepthTarget2D::m_pTexture" );

    // 深度ステンシルビューの設定.
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
    ZeroMemory( &dsvd, sizeof( D3D11_DEPTH_STENCIL_VIEW_DESC ) );
    dsvd.Format = desc.Format;

    // 配列サイズが1の場合.
    if ( desc.ArraySize == 1 )
    {
        // マルチサンプリング無しの場合.
        if ( desc.SampleDesc.Count <= 1 )
        {
            dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvd.Texture2D.MipSlice = 0;
        }
        // マルチサンプリング有りの場合.
        else
        {
            dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }
    }
    // 配列サイズが1より大きい場合.
    else
    {
        // マルチサンプリング無しの場合.
        if ( desc.SampleDesc.Count <= 1 )
        {
            dsvd.ViewDimension                  = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvd.Texture2DArray.ArraySize       = desc.ArraySize;
            dsvd.Texture2DArray.FirstArraySlice = 0;
            dsvd.Texture2DArray.MipSlice        = 0;
        }
        // マルチサンプリング有りの場合.
        else
        {
            dsvd.ViewDimension                    = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
            dsvd.Texture2DMSArray.ArraySize       = desc.ArraySize;
            dsvd.Texture2DMSArray.FirstArraySlice = 0;
        }
    }

    // 深度ステンシルビューを生成.
    hr = pDevice->CreateDepthStencilView( m_pTexture.GetPtr(), &dsvd, m_pDSV.GetAddress() );
    if ( FAILED( hr ) )
    {
        // テクスチャを解放.
        m_pTexture.Reset();

        // エラーログを出力.
        ELOG( "Error : ID3D11Device::CreateDepthStencilView() Failed. call from DepthTarget::Create()" );

        // 失敗.
        return false;
    }
    //SetDebugObjectName( m_pDSV.GetPtr(), "DepthTarget2D::m_pDSV" );

    // シェーダリソースビューの設定.
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = resourceFormat;

    // 配列サイズが1の場合.
    if ( desc.ArraySize == 1 )
    {
        // マルチサンプリング無しの場合.
        if ( desc.SampleDesc.Count <= 1 )
        {
            srvd.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvd.Texture2D.MipLevels       = desc.MipLevels;
            srvd.Texture2D.MostDetailedMip = 0;
        }
        // マルチサンプリング有りの場合.
        else
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        }
    }
    // 配列サイズが1より大きい場合.
    else
    {
        // マルチサンプリング無しの場合.
        if ( desc.SampleDesc.Count <= 1 )
        {
            srvd.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            srvd.Texture2DArray.ArraySize       = desc.ArraySize;
            srvd.Texture2DArray.FirstArraySlice = 0;
            srvd.Texture2DArray.MipLevels       = desc.MipLevels;
            srvd.Texture2DArray.MostDetailedMip = 0;
        }
        // マルチサンプリング有りの場合
        else
        {
            srvd.ViewDimension                    = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
            srvd.Texture2DMSArray.ArraySize       = desc.ArraySize;
            srvd.Texture2DMSArray.FirstArraySlice = 0;
        }
    }

    {
        // シェーダリソースビューを生成.
        hr = pDevice->CreateShaderResourceView( m_pTexture.GetPtr(), &srvd, m_pSRV.GetAddress() );
        if ( FAILED( hr ) )
        {
            // テクスチャを解放.
            m_pTexture.Reset();

            // 深度ステンシルビューを解放.
            m_pDSV.Reset();

            // エラーログを出力.
            ELOG( "Error : ID3D11Device::CreateShaderResourceView() Failed. call from DepthTarget::Create()" );

            // 失敗.
            return false;
        }
        //SetDebugObjectName( m_pSRV.GetPtr(), "DepthTarget2D::m_pSRV" );
    }

    m_Desc = desc;

    // 成功.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      リサイズ処理を行います.
//-------------------------------------------------------------------------------------------------
bool DepthTarget2D::Resize(ID3D11Device* pDevice, uint32_t width, uint32_t height)
{
    // サイズが同じ場合は変更せずに成功扱いにする.
    if (m_Desc.Width == width && m_Desc.Height == height)
    { return true; }

    m_pSRV.Reset();
    m_pDSV.Reset();
    m_pTexture.Reset();

    m_Desc.Width  = width;
    m_Desc.Height = height;

    return Create(pDevice, m_Desc);
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetDesc2D DepthTarget2D::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void DepthTarget2D::Release()
{
    m_pSRV.Reset();
    m_pDSV.Reset();
    m_pTexture.Reset();

    m_Desc = TargetDesc2D();
}

//-------------------------------------------------------------------------------------------------
//      テクスチャを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Texture2D* const DepthTarget2D::GetResource() const
{ return m_pTexture.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* const DepthTarget2D::GetShaderResource() const
{ return m_pSRV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11DepthStencilView* const DepthTarget2D::GetTargetView() const
{ return m_pDSV.GetPtr(); }


} // namespace asdx

