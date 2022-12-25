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
                    pDeviceContext->UpdateSubresource( pTexture, 0, nullptr, pInitData[0].pSysMem, pInitData[0].SysMemPitch, pInitData[0].SysMemSlicePitch );
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

//-----------------------------------------------------------------------------
//      文字列を取得します.
//-----------------------------------------------------------------------------
const char* GetString(DXGI_FORMAT format)
{
    switch (format)
    {
    default:
    case DXGI_FORMAT_UNKNOWN:
        return "UNKNOWN";

    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        return "R32G32B32A32_TYPELESS";

    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return "R32G32B32A32_FLOAT";

    case DXGI_FORMAT_R32G32B32A32_UINT:
        return "R32G32B32A32_UINT";

    case DXGI_FORMAT_R32G32B32A32_SINT:
        return "R32G32B32A32_SINT";

    case DXGI_FORMAT_R32G32B32_TYPELESS:
        return "R32G32B32_TYPELESS";

    case DXGI_FORMAT_R32G32B32_FLOAT:
        return "R32G32B32_FLOAT";

    case DXGI_FORMAT_R32G32B32_UINT:
        return "R32G32B32_UINT";

    case DXGI_FORMAT_R32G32B32_SINT:
        return "R32G32B32_SINT";

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        return "R16G16B16A16_TYPELESS";

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return "R16G16B16A16_FLOAT";

    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return "R16G16B16A16_UNORM";

    case DXGI_FORMAT_R16G16B16A16_UINT:
        return "R16G16B16A16_UINT";

    case DXGI_FORMAT_R16G16B16A16_SNORM:
        return "R16G16B16A16_SNORM";

    case DXGI_FORMAT_R16G16B16A16_SINT:
        return "R16G16B16A16_SINT";

    case DXGI_FORMAT_R32G32_TYPELESS:
        return "R32G32_TYPELESS";

    case DXGI_FORMAT_R32G32_FLOAT:
        return "R32G32_FLOAT";

    case DXGI_FORMAT_R32G32_UINT:
        return "R32G32_UINT";

    case DXGI_FORMAT_R32G32_SINT:
        return "R32G32_SINT";

    case DXGI_FORMAT_R32G8X24_TYPELESS:
        return "R32G8X24_TYPELESS";

    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        return "D32_FLOAT_S8X24_UINT";

    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        return "R32_FLOAT_X8X24_TYPELESS";

    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return "X32_TYPELESS_G8X24_UINT";

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        return "R10G10B10A2_TYPELESS";

    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return "R10G10B10A2_UNORM";

    case DXGI_FORMAT_R10G10B10A2_UINT:
        return "R10G10B10A2_UINT";

    case DXGI_FORMAT_R11G11B10_FLOAT:
        return "R11G11B10_FLOAT";

    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        return "R8G8B8A8_TYPELESS";

    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return "R8G8B8A8_UNORM";

    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return "R8G8B8A8_UNORM_SRGB";

    case DXGI_FORMAT_R8G8B8A8_UINT:
        return "R8G8B8A8_UINT";

    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return "R8G8B8A8_SNORM";

    case DXGI_FORMAT_R8G8B8A8_SINT:
        return "R8G8B8A8_SINT";

    case DXGI_FORMAT_R16G16_TYPELESS:
        return "R16G16_TYPELESS";

    case DXGI_FORMAT_R16G16_FLOAT:
        return "R16G16_FLOAT";

    case DXGI_FORMAT_R16G16_UNORM:
        return "R16G16_UNORM";

    case DXGI_FORMAT_R16G16_UINT:
        return "R16G16_UINT";

    case DXGI_FORMAT_R16G16_SNORM:
        return "R16G16_SNORM";

    case DXGI_FORMAT_R16G16_SINT:
        return "R16G16_SINT";

    case DXGI_FORMAT_R32_TYPELESS:
        return "R32_TYPELESS";

    case DXGI_FORMAT_D32_FLOAT:
        return "D32_FLOAT";

    case DXGI_FORMAT_R32_FLOAT:
        return "R32_FLOAT";

    case DXGI_FORMAT_R32_UINT:
        return "R32_UINT";

    case DXGI_FORMAT_R32_SINT:
        return "R32_SINT";

    case DXGI_FORMAT_R24G8_TYPELESS:
        return "R24G8_TYPELESS";

    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        return "D24_UNORM_S8_UINT";

    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        return "R24_UNORM_X8_TYPELESS";

    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        return "X24_TYPELESS_G8_UINT";

    case DXGI_FORMAT_R8G8_TYPELESS:
        return "R8G8_TYPELESS";

    case DXGI_FORMAT_R8G8_UNORM:
        return "R8G8_UNORM";

    case DXGI_FORMAT_R8G8_UINT:
        return "R8G8_UINT";

    case DXGI_FORMAT_R8G8_SNORM:
        return "R8G8_SNORM";

    case DXGI_FORMAT_R8G8_SINT:
        return "R8G8_SINT";

    case DXGI_FORMAT_R16_TYPELESS:
        return "R16_TYPELESS";

    case DXGI_FORMAT_R16_FLOAT:
        return "R16_FLOAT";

    case DXGI_FORMAT_D16_UNORM:
        return "D16_UNORM";

    case DXGI_FORMAT_R16_UNORM:
        return "R16_UNORM";

    case DXGI_FORMAT_R16_UINT:
        return "R16_UINT";

    case DXGI_FORMAT_R16_SNORM:
        return "R16_SNORM";

    case DXGI_FORMAT_R16_SINT:
        return "R16_SINT";

    case DXGI_FORMAT_R8_TYPELESS:
        return "R8_TYPELESS";

    case DXGI_FORMAT_R8_UNORM:
        return "R8_UNORM";

    case DXGI_FORMAT_R8_UINT:
        return "R8_UINT";

    case DXGI_FORMAT_R8_SNORM:
        return "R8_SNORM";

    case DXGI_FORMAT_R8_SINT:
        return "R8_SINT";

    case DXGI_FORMAT_A8_UNORM:
        return "A8_UNORM";

    case DXGI_FORMAT_R1_UNORM:
        return "R1_UNORM";

    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        return "R9G9B9E5_SHAREDEXP";

    case DXGI_FORMAT_R8G8_B8G8_UNORM:
        return "R8G8_B8G8_UNORM";

    case DXGI_FORMAT_G8R8_G8B8_UNORM:
        return "G8R8_G8B8_UNORM";

    case DXGI_FORMAT_BC1_TYPELESS:
        return "BC1_TYPELESS";

    case DXGI_FORMAT_BC1_UNORM:
        return "BC1_UNORM";

    case DXGI_FORMAT_BC1_UNORM_SRGB:
        return "BC1_UNORM_SRGB";

    case DXGI_FORMAT_BC2_TYPELESS:
        return "BC2_TYPELESS";

    case DXGI_FORMAT_BC2_UNORM:
        return "BC2_UNORM";

    case DXGI_FORMAT_BC2_UNORM_SRGB:
        return "BC2_UNORM_SRGB";

    case DXGI_FORMAT_BC3_TYPELESS:
        return "BC3_TYPELESS";

    case DXGI_FORMAT_BC3_UNORM:
        return "BC3_UNORM";

    case DXGI_FORMAT_BC3_UNORM_SRGB:
        return "BC3_UNORM_SRGB";

    case DXGI_FORMAT_BC4_TYPELESS:
        return "BC4_TYPELESS";

    case DXGI_FORMAT_BC4_UNORM:
        return "BC4_UNORM";

    case DXGI_FORMAT_BC4_SNORM:
        return "BC4_SNORM";

    case DXGI_FORMAT_BC5_TYPELESS:
        return "BC5_TYPELESS";

    case DXGI_FORMAT_BC5_UNORM:
        return "BC5_UNORM";

    case DXGI_FORMAT_BC5_SNORM:
        return "BC5_SNORM";

    case DXGI_FORMAT_B5G6R5_UNORM:
        return "B5G6R5_UNORM";

    case DXGI_FORMAT_B5G5R5A1_UNORM:
        return "B5G5R5A1_UNORM";

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return "B8G8R8A8_UNORM";

    case DXGI_FORMAT_B8G8R8X8_UNORM:
        return "B8G8R8X8_UNORM";

    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        return "R10G10B10_XR_BIAS_A2_UNORM";

    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        return "B8G8R8A8_TYPELESS";

    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return "B8G8R8A8_UNORM_SRGB";

    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        return "B8G8R8X8_TYPELESS";

    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return "B8G8R8X8_UNORM_SRGB";

    case DXGI_FORMAT_BC6H_TYPELESS:
        return "BC6H_TYPELESS";

    case DXGI_FORMAT_BC6H_UF16:
        return "BC6H_UF16";

    case DXGI_FORMAT_BC6H_SF16:
        return "BC6H_SF16";

    case DXGI_FORMAT_BC7_TYPELESS:
        return "BC7_TYPELESS";

    case DXGI_FORMAT_BC7_UNORM:
        return "BC7_UNORM";

    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return "BC7_UNORM_SRGB";

    case DXGI_FORMAT_AYUV:
        return "AYUV";

    case DXGI_FORMAT_Y410:
        return "Y410";

    case DXGI_FORMAT_Y416:
        return "Y416";

    case DXGI_FORMAT_NV12:
        return "NV12";

    case DXGI_FORMAT_P010:
        return "P010";

    case DXGI_FORMAT_P016:
        return "P016";

    case DXGI_FORMAT_420_OPAQUE:
        return "420_OPAQUE";

    case DXGI_FORMAT_YUY2:
        return "YUY2";

    case DXGI_FORMAT_Y210:
        return "Y210";

    case DXGI_FORMAT_Y216:
        return "Y216";

    case DXGI_FORMAT_NV11:
        return "NV11";

    case DXGI_FORMAT_AI44:
        return "AI44";

    case DXGI_FORMAT_IA44:
        return "IA44";

    case DXGI_FORMAT_P8:
        return "P8";

    case DXGI_FORMAT_A8P8:
        return "A8P8";

    case DXGI_FORMAT_B4G4R4A4_UNORM:
        return "B4G4R4A4_UNORM";

    case DXGI_FORMAT_P208:
        return "P208";

    case DXGI_FORMAT_V208:
        return "V208";

    case DXGI_FORMAT_V408:
        return "V408";

    case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
        return "SAMPLER_FEEDBACK_MIN_MIP_OPAQUE";

    case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
        return "SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE";
    }
}


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
    m_Format = DXGI_FORMAT((resource.Option & SUBRESOURCE_OPTION_SRGB) ? asdx::MakeSRGB(resource.Format) : resource.Format);

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
    m_Format = DXGI_FORMAT((resource.Option & SUBRESOURCE_OPTION_SRGB) ? asdx::MakeSRGB(resource.Format) : resource.Format);

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
