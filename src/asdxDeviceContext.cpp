//-----------------------------------------------------------------------------
// File : asdxDeviceContext.cpp
// Desc : Device Context.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxDeviceContext.h>
#include <asdxLogger.h>
#if defined(ASDX_ENABLE_D3D11ON12)
#include <d3d11on12.h>
#endif


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// DeviceContext class
///////////////////////////////////////////////////////////////////////////////
const float DeviceContext::kDefaultBlendFactor[4] = {
    D3D11_DEFAULT_BLEND_FACTOR_RED,
    D3D11_DEFAULT_BLEND_FACTOR_GREEN,
    D3D11_DEFAULT_BLEND_FACTOR_BLUE,
    D3D11_DEFAULT_BLEND_FACTOR_ALPHA
};
DeviceContext DeviceContext::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
DeviceContext::DeviceContext()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
DeviceContext::~DeviceContext()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool DeviceContext::Init()
{
#if ASDX_ENABLE_D3D11ON12
    if (!InitD3D12())
    {
        ELOG("Error : InitD3D12() Failed.");
        return false;
    }

    IUnknown* commandQueues[] = {
        m_D3D12GraphicsQueue.GetPtr()
    };
#endif

    // デバイス生成フラグ.
    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif//defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    // ドライバータイプ.
    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT driverTypeCount = _countof(driverTypes);

    // 機能レベル.
    D3D_FEATURE_LEVEL featureLevels[] = {
        //D3D_FEATURE_LEVEL_11_1,       // へぼPC向け対応 [ D3D11.1に対応していないハードウェアでは，問答無用で落とされる場合があるので仕方なく除外. ]
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };
    UINT featureLevelCount = _countof(featureLevels);

    HRESULT hr = S_OK;

    RefPtr<ID3D11Device>        device;
    RefPtr<ID3D11DeviceContext> context;

    for(auto i=0u; i<driverTypeCount; ++i)
    {
        m_DriverType = driverTypes[i];

#if ASDX_ENABLE_D3D11ON12
        hr = D3D11On12CreateDevice(
            m_D3D12Device.GetPtr(),
            createDeviceFlags,
            featureLevels,
            featureLevelCount,
            commandQueues,
            _countof(commandQueues),
            0,
            device.GetAddress(),
            context.GetAddress(),
            &m_FeatureLevel);
#else
        hr = D3D11CreateDevice(
            nullptr,
            m_DriverType,
            nullptr,
            createDeviceFlags,
            featureLevels,
            featureLevelCount,
            D3D11_SDK_VERSION,
            device.GetAddress(),
            &m_FeatureLevel,
            context.GetAddress());
#endif

        if (SUCCEEDED(hr))
        { break; }
    }

    if (FAILED(hr))
    {
        ELOG("Error : D3D11CreateDevice() Failed. errcode = 0x%x", hr);
        return false;
    }

    hr = device->QueryInterface(IID_PPV_ARGS(m_pDevice.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

#if defined(DEBUG) || defined(_DEBUG)
    hr = m_pDevice->QueryInterface(IID_PPV_ARGS(m_pInfoQueue.GetAddress()));
    if (SUCCEEDED(hr))
    {
        // エラー発生時にブレークさせる.
        m_pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);

        // 警告発生時にブレークさせる.
        m_pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
    }
#endif

    hr = context->QueryInterface(IID_PPV_ARGS(m_pContext.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11DeviceContext::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

    // DXGIデバイスを取得.
    hr = m_pDevice->QueryInterface(IID_PPV_ARGS(m_DXGIDevice.GetAddress()));
    if ( FAILED( hr ) )
    {
        ELOG("Error : ID3D11Device::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

    // DXGIアダプター取得.
    hr = m_DXGIDevice->GetAdapter(m_DXGIAdapter.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIDevice::GetAdapter() Failed. errcode = 0x%x", hr);
        return false;
    }

    // DXGIファクトリー取得.
    hr = m_DXGIAdapter->GetParent(IID_PPV_ARGS(m_DXGIFactory.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIAdapter::GetParent() Failed. errcode = 0x%x", hr);
        return false;
    }

    // ユーザーアノテーションを取得.
    hr = m_pContext->QueryInterface(IID_PPV_ARGS(m_Annotation.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11DeviceContext::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

    // ラスタライザーステートを生成.
    {
        D3D11_RASTERIZER_DESC desc = {};
        desc.FillMode              = D3D11_FILL_SOLID;
        desc.CullMode              = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthBias             = D3D11_DEFAULT_DEPTH_BIAS;
        desc.DepthBiasClamp        = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        desc.DepthClipEnable       = FALSE;
        desc.SlopeScaledDepthBias  = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        desc.ScissorEnable         = TRUE;
        desc.MultisampleEnable     = FALSE;
        desc.AntialiasedLineEnable = FALSE;

        // ラスタライザーステートを生成.
        hr = m_pDevice->CreateRasterizerState( &desc, m_DefaultRS.GetAddress() );
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateRasterizerState() Failed. errcode = 0x%x", hr);
            return false;
        }

        // デバイスコンテキストにラスタライザーステートを設定.
        m_pContext->RSSetState( m_DefaultRS.GetPtr() );
    }

    // 深度ステンシルステートを生成.
    {
        const D3D11_DEPTH_STENCILOP_DESC defaultStencilOp =
        {
            D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP_KEEP,
            D3D11_COMPARISON_ALWAYS
        };

        D3D11_DEPTH_STENCIL_DESC desc = {};
        desc.DepthEnable        = TRUE;
        desc.DepthWriteMask     = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc          = D3D11_COMPARISON_LESS;
        desc.StencilEnable      = FALSE;
        desc.StencilReadMask    = D3D11_DEFAULT_STENCIL_READ_MASK;
        desc.StencilWriteMask   = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        desc.FrontFace          = defaultStencilOp;
        desc.BackFace           = defaultStencilOp;

        hr = m_pDevice->CreateDepthStencilState(&desc, m_DefaultDSS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateDepthStencilState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // ブレンドステートを生成.
    {
        const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
        {
            FALSE,
            D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
            D3D11_COLOR_WRITE_ENABLE_ALL,
        };

        D3D11_BLEND_DESC desc = {};
        desc.AlphaToCoverageEnable  = FALSE;
        desc.IndependentBlendEnable = FALSE;
        for (auto i=0u; i<D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        { desc.RenderTarget[ i ] = defaultRenderTargetBlendDesc; }

        hr = m_pDevice->CreateBlendState(&desc, m_DefaultBS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateBlendState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void DeviceContext::Term()
{
    if (m_pContext.GetPtr() != nullptr)
    { m_pContext->Flush(); }

    m_pInfoQueue    .Reset();
    m_DefaultBS     .Reset();
    m_DefaultDSS    .Reset();
    m_DefaultRS     .Reset();
    m_pContext      .Reset();
    m_DXGIFactory   .Reset();
    m_DXGIAdapter   .Reset();
    m_DXGIDevice    .Reset();
    m_Annotation    .Reset();
    m_pDevice       .Reset();

#if ASDX_ENABLE_D3D11ON12
    TermD3D12();
#endif
}

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
DeviceContext& DeviceContext::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      デバイスを取得します.
//-----------------------------------------------------------------------------
ID3D11Device* DeviceContext::GetDevice() const
{ return m_pDevice.GetPtr(); }

//-----------------------------------------------------------------------------
//      イミディエイトコンテキストを取得します.
//-----------------------------------------------------------------------------
ID3D11DeviceContext4* DeviceContext::GetContext() const
{ return m_pContext.GetPtr(); }

//-----------------------------------------------------------------------------
//      ラスタライザーステートを取得します.
//-----------------------------------------------------------------------------
ID3D11RasterizerState* DeviceContext::GetDefaultRS() const
{ return m_DefaultRS.GetPtr(); }

//-----------------------------------------------------------------------------
//      深度ステンシルステートを取得します.
//-----------------------------------------------------------------------------
ID3D11DepthStencilState* DeviceContext::GetDefaultDSS() const
{ return m_DefaultDSS.GetPtr(); }

//-----------------------------------------------------------------------------
//      ブレンドステートを取得します.
//-----------------------------------------------------------------------------
ID3D11BlendState* DeviceContext::GetDefaultBS() const
{ return m_DefaultBS.GetPtr(); }

//-----------------------------------------------------------------------------
//      DXGIデバイスを取得します.
//-----------------------------------------------------------------------------
IDXGIDevice* DeviceContext::GetDXGIDevice() const
{ return m_DXGIDevice.GetPtr(); }

//-----------------------------------------------------------------------------
//      DXGIアダプターを取得します.
//-----------------------------------------------------------------------------
IDXGIAdapter* DeviceContext::GetDXGIAdapter() const
{ return m_DXGIAdapter.GetPtr(); }

//-----------------------------------------------------------------------------
//      DXGIファクトリーを取得します.
//-----------------------------------------------------------------------------
IDXGIFactory2* DeviceContext::GetDXGIFactory() const
{ return m_DXGIFactory.GetPtr(); }

//-----------------------------------------------------------------------------
//      ユーザーアノテーションを取得します.
//-----------------------------------------------------------------------------
ID3DUserDefinedAnnotation* DeviceContext::GetAnnotation() const
{ return m_Annotation.GetPtr(); }

//-----------------------------------------------------------------------------
//      ドライバータイプを取得します.
//-----------------------------------------------------------------------------
D3D_DRIVER_TYPE DeviceContext::GetDriverType() const
{ return m_DriverType; }

//-----------------------------------------------------------------------------
//      機能レベルを取得します.
//-----------------------------------------------------------------------------
D3D_FEATURE_LEVEL DeviceContext::GetFeatureLevel() const
{ return m_FeatureLevel; }

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D11DeviceContext* DeviceContext::operator->() const
{ return m_pContext.GetPtr(); }

#if ASDX_ENABLE_D3D11ON12
//-----------------------------------------------------------------------------
//      D3D12の初期化処理.
//-----------------------------------------------------------------------------
bool DeviceContext::InitD3D12()
{
    auto hr = D3D12CreateDevice(
        nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_D3D12Device.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : D3D12CreateDevice() Failed. errcode = 0x%x", hr);
        return false;
    }

#if defined(DEBUG) || defined(_DEBUG)
    // ID3D12InfoQueueに変換.
    {
        hr = m_D3D12Device->QueryInterface(IID_PPV_ARGS(m_D3D12InfoQueue.GetAddress()));
        if (SUCCEEDED(hr))
        {
            // エラー発生時にブレークさせる.
            m_D3D12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

            // 警告発生時にブレークさせる.
            m_D3D12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            // 無視するメッセージID.
            D3D12_MESSAGE_ID denyIds[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
            };

            // 無視するメッセージレベル.
            D3D12_MESSAGE_SEVERITY severities[] = {
                D3D12_MESSAGE_SEVERITY_INFO
            };

            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs          = _countof(denyIds);
            filter.DenyList.pIDList         = denyIds;
            filter.DenyList.NumSeverities   = _countof(severities);
            filter.DenyList.pSeverityList   = severities;

            m_D3D12InfoQueue->PushStorageFilter(&filter);
        }
    }
#endif

    // グラフィックスキューの生成.
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type       = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Priority   = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags      = D3D12_COMMAND_QUEUE_FLAG_NONE;

        hr = m_D3D12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_D3D12GraphicsQueue.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommandQueue() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      D3D12の終了処理です.
//-----------------------------------------------------------------------------
void DeviceContext::TermD3D12()
{
    m_D3D12GraphicsQueue.Reset();
    m_D3D12InfoQueue    .Reset();
    m_D3D12Device       .Reset();
}

//-----------------------------------------------------------------------------
//      D3D12デバイスを取得します.
//-----------------------------------------------------------------------------
ID3D12Device* DeviceContext::GetD3D12Device() const
{ return m_D3D12Device.GetPtr(); }

//-----------------------------------------------------------------------------
//      D3D12グラフィックスキューを取得します.
//-----------------------------------------------------------------------------
ID3D12CommandQueue* DeviceContext::GetD3D12GraphicsQueue() const
{ return m_D3D12GraphicsQueue.GetPtr(); }

#endif

} // namespace asdx
