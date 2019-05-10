//-------------------------------------------------------------------------------------------------
// File : asdxRenderState.cpp
// Desc : Render State Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxRenderState.h>
#include <asdxMisc.h>
#include <asdxLogger.h>
#include <cassert>


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      深度ステンシルステートを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateDSS
(
    ID3D11Device* pDevice,
    bool enableTest,
    bool enableWrite,
    asdx::RefPtr<ID3D11DepthStencilState>& result
)
{
    D3D11_DEPTH_STENCIL_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );

    desc.DepthEnable                  = ( !enableTest && enableWrite ) ? TRUE : enableTest;
    desc.DepthWriteMask               = enableWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc                    = ( !enableTest && enableWrite ) ? D3D11_COMPARISON_ALWAYS : D3D11_COMPARISON_LESS_EQUAL;
    desc.StencilEnable                = FALSE;
    desc.StencilReadMask              = D3D11_DEFAULT_STENCIL_READ_MASK;
    desc.StencilWriteMask             = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    desc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    desc.BackFace                     = desc.FrontFace;

    ID3D11DepthStencilState* pDSS;
    HRESULT hr = pDevice->CreateDepthStencilState( &desc, &pDSS );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D11Device::CreateDepthStencilState() Failed." );
        return false;
    }

    result.Attach( pDSS );
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ラスタライザーステートを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateRS
(
    ID3D11Device* pDevice,
    D3D11_CULL_MODE cullMode,
    D3D11_FILL_MODE fillMode,
    bool enableMS,
    asdx::RefPtr<ID3D11RasterizerState>& result
)
{
    D3D11_RASTERIZER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );

    desc.CullMode          = cullMode;
    desc.FillMode          = fillMode;
    desc.DepthClipEnable   = true;
    desc.ScissorEnable     = true;
    desc.MultisampleEnable = enableMS;

    ID3D11RasterizerState* pRS;
    HRESULT hr = pDevice->CreateRasterizerState( &desc, &pRS );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D11Device::CreateRasterizerState() Failed." );
        return false;
    }

    result.Attach( pRS );
    return true;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーステートを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateSmp
(
    ID3D11Device* pDevice,
    D3D11_FILTER filter,
    D3D11_TEXTURE_ADDRESS_MODE addressMode,
    asdx::RefPtr<ID3D11SamplerState>& result
)
{
    D3D11_SAMPLER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );

    desc.Filter         = filter;
    desc.AddressU       = addressMode;
    desc.AddressV       = addressMode;
    desc.AddressW       = addressMode;
    desc.MaxAnisotropy  = ( pDevice->GetFeatureLevel() > D3D_FEATURE_LEVEL_9_1 ) ? 16 : 2;
    desc.MaxLOD         = FLT_MAX;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    ID3D11SamplerState* pSmp;
    HRESULT hr = pDevice->CreateSamplerState( &desc, &pSmp );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D11Device::CreateSamplerState() Failed." );
        return false;
    }

    result.Attach( pSmp );
    return true;
}

//-------------------------------------------------------------------------------------------------
//      比較用サンプラーステートを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateCmpSmp
(
    ID3D11Device* pDevice,
    D3D11_COMPARISON_FUNC cmpFunc,
    D3D11_FILTER filter,
    D3D11_TEXTURE_ADDRESS_MODE addressMode,
    asdx::RefPtr<ID3D11SamplerState>& result
)
{
    D3D11_SAMPLER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );

    desc.Filter         = filter;
    desc.AddressU       = addressMode;
    desc.AddressV       = addressMode;
    desc.AddressW       = addressMode;
    desc.MaxAnisotropy  = ( pDevice->GetFeatureLevel() > D3D_FEATURE_LEVEL_9_1 ) ? 16 : 2;
    desc.MaxLOD         = FLT_MAX;
    desc.ComparisonFunc = cmpFunc;
    desc.BorderColor[0] = 1.0f;
    desc.BorderColor[1] = 1.0f;
    desc.BorderColor[2] = 1.0f;
    desc.BorderColor[3] = 1.0f;

    ID3D11SamplerState* pSmp;
    HRESULT hr = pDevice->CreateSamplerState( &desc, &pSmp );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D11Device::CreateSamplerState() Failed." );
        return false;
    }

    result.Attach( pSmp );
    return true;
}


} // namespace /* anonymous */


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderState class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
RenderState::RenderState()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
RenderState::~RenderState()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      唯一のインスタンスを取得します.
//-------------------------------------------------------------------------------------------------
RenderState& RenderState::GetInstance()
{
    static RenderState instance;
    return instance;
}

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool RenderState::Init( ID3D11Device* pDevice )
{
    if ( m_IsInit )
    { return true; }

    if (!pDevice)
    {
        ELOG("Error : Invalid Argument");
        return false;
    }


    // ブレンドステート.
    {
        for ( auto i = 0; i < BlendType::NumBlendType; ++i )
        {
            D3D11_BLEND_DESC desc = {};
            desc.AlphaToCoverageEnable  = FALSE;
            desc.IndependentBlendEnable = FALSE;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            switch(i)
            {
            case BlendType::Opaque:
                {
                    desc.RenderTarget[0].BlendEnable = FALSE;
                    desc.RenderTarget[0].SrcBlend    = desc.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_ONE;
                    desc.RenderTarget[0].DestBlend   = desc.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ZERO;
                    desc.RenderTarget[0].BlendOp     = desc.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
                }
                break;

            case BlendType::AlphaBlend:
                {
                    desc.RenderTarget[0].BlendEnable = TRUE;
                    desc.RenderTarget[0].SrcBlend    = desc.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_SRC_ALPHA;
                    desc.RenderTarget[0].DestBlend   = desc.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_INV_SRC_ALPHA;
                    desc.RenderTarget[0].BlendOp     = desc.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
                }
                break;

            case BlendType::Additive:
                {
                    desc.RenderTarget[0].BlendEnable = TRUE;
                    desc.RenderTarget[0].SrcBlend    = desc.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_SRC_ALPHA;
                    desc.RenderTarget[0].DestBlend   = desc.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ONE;
                    desc.RenderTarget[0].BlendOp     = desc.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
                }
                break;

            case BlendType::Subtract:
                {
                    desc.RenderTarget[0].BlendEnable = TRUE;
                    desc.RenderTarget[0].SrcBlend    = desc.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_SRC_ALPHA;
                    desc.RenderTarget[0].DestBlend   = desc.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ONE;
                    desc.RenderTarget[0].BlendOp     = desc.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_REV_SUBTRACT;
                }
                break;

            case BlendType::Premultiplied:
                {
                    desc.RenderTarget[0].BlendEnable = TRUE;
                    desc.RenderTarget[0].SrcBlend    = desc.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_ONE;
                    desc.RenderTarget[0].DestBlend   = desc.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_INV_SRC_ALPHA;
                    desc.RenderTarget[0].BlendOp     = desc.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
                }
                break;

            case BlendType::Multiply:
                {
                    desc.RenderTarget[0].BlendEnable    = TRUE;
                    desc.RenderTarget[0].SrcBlend       = desc.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_ZERO;
                    desc.RenderTarget[0].DestBlend      = D3D11_BLEND_SRC_COLOR;
                    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;
                    desc.RenderTarget[0].BlendOp        = desc.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
                }
                break;

            case BlendType::Screen:
                {
                    desc.RenderTarget[0].BlendEnable    = TRUE;
                    desc.RenderTarget[0].SrcBlend       = D3D11_BLEND_DEST_COLOR;
                    desc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_DEST_ALPHA;
                    desc.RenderTarget[0].DestBlend      = desc.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ONE;
                    desc.RenderTarget[0].BlendOp        = desc.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
                }
                break;
            }

            auto hr = pDevice->CreateBlendState(&desc, m_BS[i].GetAddress());
            if (FAILED(hr))
            {
                ELOG("Error : ID3D11Device::CreateBlendState() Failed.");
                Term();
                return false;
            }
        }
    }

    // 深度ステンシルステート.
    {
        struct ArgDSS
        {
            DepthType type;
            bool      depthTest;
            bool      depthWrite;
        };

        ArgDSS args[] = {
            { DepthType::None,     false, false },
            { DepthType::Default,  true,  true  },
            { DepthType::Readonly, true,  false },
            { DepthType::WriteOnly,false, true  }
        };

        for ( auto i = 0; i < NumDepthType; ++i )
        {
            if ( !CreateDSS( pDevice, args[ i ].depthTest, args[ i ].depthWrite, m_DSS[ args[ i ].type ] ) )
            {
                ELOG( "Error : CreateDSS() Failed. index = %d", i );
                Term();
                return false;
            }
        }
    }

    // ラスタライザーステート.
    {
        struct ArgRS
        {
            RasterizerType  type;
            D3D11_CULL_MODE cull;
            D3D11_FILL_MODE fill;
            bool            multiSample;
        };

        ArgRS args[] = {
            { RasterizerType::CullNone,                 D3D11_CULL_NONE,    D3D11_FILL_SOLID,       false },
            { RasterizerType::CullNoneMS,               D3D11_CULL_NONE,    D3D11_FILL_SOLID,       true  },
            { RasterizerType::CullClockWise,            D3D11_CULL_FRONT,   D3D11_FILL_SOLID,       false },
            { RasterizerType::CullClockWiseMS,          D3D11_CULL_FRONT,   D3D11_FILL_SOLID,       true  },
            { RasterizerType::CullCounterClockWise,     D3D11_CULL_BACK,    D3D11_FILL_SOLID,       false },
            { RasterizerType::CullCounterClockWiseMS,   D3D11_CULL_BACK,    D3D11_FILL_SOLID,       true  },
            { RasterizerType::WireFrame,                D3D11_CULL_NONE,    D3D11_FILL_WIREFRAME,   false },
            { RasterizerType::WireFrameMS,              D3D11_CULL_NONE,    D3D11_FILL_WIREFRAME,   true  }
        };

        for ( auto i = 0; i < NumRasterizerType; ++i )
        {
            if ( !CreateRS( pDevice, args[ i ].cull, args[ i ].fill, args[ i ].multiSample, m_RS[ args[ i ].type ] ) )
            {
                ELOG( "Error : CreateRS() Failed. index = %d", i );
                Term();
                return false;
            }
        }
    }

    // サンプラーステート.
    {
        struct ArgSS
        {
            SamplerType                 type;
            D3D11_FILTER                filter;
            D3D11_TEXTURE_ADDRESS_MODE  address;
        };

        ArgSS args[] = {
            { SamplerType::PointWrap,           D3D11_FILTER_MIN_MAG_MIP_POINT,     D3D11_TEXTURE_ADDRESS_WRAP   },
            { SamplerType::PointClamp,          D3D11_FILTER_MIN_MAG_MIP_POINT,     D3D11_TEXTURE_ADDRESS_CLAMP  },
            { SamplerType::PointMirror,         D3D11_FILTER_MIN_MAG_MIP_POINT,     D3D11_TEXTURE_ADDRESS_MIRROR },
            { SamplerType::LinearWrap,          D3D11_FILTER_MIN_MAG_MIP_LINEAR,    D3D11_TEXTURE_ADDRESS_WRAP   },
            { SamplerType::LinearClamp,         D3D11_FILTER_MIN_MAG_MIP_LINEAR,    D3D11_TEXTURE_ADDRESS_CLAMP  },
            { SamplerType::LinearMirror,        D3D11_FILTER_MIN_MAG_MIP_LINEAR,    D3D11_TEXTURE_ADDRESS_MIRROR },
            { SamplerType::AnisotropicWrap,     D3D11_FILTER_ANISOTROPIC,           D3D11_TEXTURE_ADDRESS_WRAP   },
            { SamplerType::AnisotropicClamp,    D3D11_FILTER_ANISOTROPIC,           D3D11_TEXTURE_ADDRESS_CLAMP  },
            { SamplerType::AnisotropicMirror,   D3D11_FILTER_ANISOTROPIC,           D3D11_TEXTURE_ADDRESS_MIRROR },
        };

        for ( auto i = 0; i < NumSamplerType; ++i )
        {
            if ( !CreateSmp( pDevice, args[ i ].filter, args[ i ].address, m_SS[ args[ i ].type ] ) )
            {
                ELOG( "Error : CreateSmp() Failed. index = %d", i );
                Term();
                return false;
            }
        }
    }

    // 比較用サンプラーステート.
    {
        struct ArgSCS
        {
            SamplerComparisonType       type;
            D3D11_COMPARISON_FUNC       comp;
            D3D11_FILTER                filter;
            D3D11_TEXTURE_ADDRESS_MODE  address;
        };

        ArgSCS args[] = {
            { SamplerComparisonType::PointLEqual,       D3D11_COMPARISON_LESS_EQUAL,    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,  D3D11_TEXTURE_ADDRESS_BORDER },
            { SamplerComparisonType::PointGEqual,       D3D11_COMPARISON_GREATER_EQUAL, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,  D3D11_TEXTURE_ADDRESS_BORDER },
            { SamplerComparisonType::LinearLEqual,      D3D11_COMPARISON_LESS_EQUAL,    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER },
            { SamplerComparisonType::LinearGEqual,      D3D11_COMPARISON_GREATER_EQUAL, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER },
            { SamplerComparisonType::AnisotropicLEqual, D3D11_COMPARISON_LESS_EQUAL,    D3D11_FILTER_COMPARISON_ANISOTROPIC,        D3D11_TEXTURE_ADDRESS_BORDER },
            { SamplerComparisonType::AnisotropicGEqual, D3D11_COMPARISON_GREATER_EQUAL, D3D11_FILTER_COMPARISON_ANISOTROPIC,        D3D11_TEXTURE_ADDRESS_BORDER },
        };

        for ( auto i = 0; i < NumSamplerComparisonType; ++i )
        {
            if ( !CreateCmpSmp( pDevice, args[ i ].comp, args[ i ].filter, args[ i ].address, m_SCS[ args[ i ].type ] ) )
            {
                ELOG( "Error : CreateCmpSmp() Failed. index = %d", i );
                Term();
                return false;
            }
        }
    }

    // 初期化済みフラグを立てる.
    m_IsInit = true;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void RenderState::Term()
{
    for ( auto i = 0; i < NumBlendType; ++i )
    { m_BS[ i ].Reset(); }

    for ( auto i = 0; i < NumDepthType; ++i )
    { m_DSS[ i ].Reset(); }

    for ( auto i = 0; i < NumRasterizerType; ++i )
    { m_RS[ i ].Reset(); }

    for ( auto i = 0; i < NumSamplerType; ++i )
    { m_SS[ i ].Reset(); }

    for( auto i = 0; i < NumSamplerComparisonType; ++i)
    { m_SCS[ i ].Reset(); }

    m_IsInit = false;
}

//-------------------------------------------------------------------------------------------------
//      初期化済みかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool RenderState::IsInit() const
{
    return m_IsInit;
}

//-------------------------------------------------------------------------------------------------
//      ブレンドステートを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11BlendState* RenderState::GetBS( BlendType type ) const
{
    assert( 0 <= type && type < NumBlendType );
    return m_BS[ type ].GetPtr();
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルステートを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11DepthStencilState* RenderState::GetDSS( DepthType type ) const
{
    assert( 0 <= type && type < NumDepthType );
    return m_DSS[ type ].GetPtr();
}

//-------------------------------------------------------------------------------------------------
//      ラスタライザーステートを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11RasterizerState* RenderState::GetRS( RasterizerType type ) const
{
    assert( 0 <= type && type < NumRasterizerType );
    return m_RS[ type ].GetPtr();
}

//-------------------------------------------------------------------------------------------------
//      サンプラーステートを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11SamplerState* RenderState::GetSmp( SamplerType type ) const
{
    assert( 0 <= type && type < NumSamplerType );
    return m_SS[ type ].GetPtr();
}

//-------------------------------------------------------------------------------------------------
//      サンプラーコンパリソンステートを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11SamplerState* RenderState::GetSmpCmp( SamplerComparisonType type ) const
{
    assert( 0 <= type && type < NumSamplerComparisonType );
    return m_SCS[ type ].GetPtr();
}


} // namespace asdx
