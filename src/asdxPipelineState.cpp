//-----------------------------------------------------------------------------
// File : asdxPipelineState.cpp
// Desc : Pipeline State Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxPipelineState.h>
#include <asdxLogger.h>
#include <cfloat>


namespace {

#include "../res/shaders/Compiled/FullScreenVS.inc"
#include "../res/shaders/Compiled/CopyPS.inc"

static const D3D11_INPUT_ELEMENT_DESC kFullScreenElements[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

} // namespace

namespace asdx {

//-----------------------------------------------------------------------------
//      ブレンド設定を取得します.
//-----------------------------------------------------------------------------
D3D11_BLEND_DESC GetBlendDesc(BlendState type)
{
    D3D11_BLEND_DESC result = {};
    result.AlphaToCoverageEnable                 = FALSE;
    result.IndependentBlendEnable                = FALSE;
    result.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    switch(type)
    {
    case asdx::BlendState::Opaque:
        {
            result.RenderTarget[0].BlendEnable = FALSE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_ONE;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ZERO;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
        }
        break;

    case asdx::BlendState::AlphaBlend:
        {
            result.RenderTarget[0].BlendEnable = TRUE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_SRC_ALPHA;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_INV_SRC_ALPHA;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
        }
        break;

    case asdx::BlendState::Additive:
        {
            result.RenderTarget[0].BlendEnable = TRUE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_SRC_ALPHA;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ONE;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
        }
        break;

    case asdx::BlendState::Subtract:
        {
            result.RenderTarget[0].BlendEnable = TRUE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_SRC_ALPHA;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ONE;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_REV_SUBTRACT;
        }
        break;

    case asdx::BlendState::Premultiplied:
        {
            result.RenderTarget[0].BlendEnable = TRUE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_ONE;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_INV_SRC_ALPHA;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
        }
        break;

    case asdx::BlendState::Multiply:
        {
            result.RenderTarget[0].BlendEnable    = TRUE;
            result.RenderTarget[0].SrcBlend       = result.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_ZERO;
            result.RenderTarget[0].DestBlend      = D3D11_BLEND_SRC_COLOR;
            result.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;
            result.RenderTarget[0].BlendOp        = result.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
        }
        break;

    case asdx::BlendState::Screen:
        {
            result.RenderTarget[0].BlendEnable    = TRUE;
            result.RenderTarget[0].SrcBlend       = D3D11_BLEND_DEST_COLOR;
            result.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_DEST_ALPHA;
            result.RenderTarget[0].DestBlend      = result.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ONE;
            result.RenderTarget[0].BlendOp        = result.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
        }
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      深度ステンシルステートを取得します.
//-----------------------------------------------------------------------------
D3D11_DEPTH_STENCIL_DESC GetDepthStencilState(DepthState type, D3D11_COMPARISON_FUNC compare)
{
    BOOL depthEnable;
    D3D11_DEPTH_WRITE_MASK depthWriteMask;

    switch(type)
    {
    case DepthState::Default:
    default:
        depthEnable     = TRUE;
        depthWriteMask  = D3D11_DEPTH_WRITE_MASK_ALL;
        break;

    case DepthState::None:
        depthEnable     = FALSE;
        depthWriteMask  = D3D11_DEPTH_WRITE_MASK_ZERO;
        break;

    case DepthState::ReadOnly:
        depthEnable     = TRUE;
        depthWriteMask  = D3D11_DEPTH_WRITE_MASK_ZERO;
        break;

    case DepthState::WriteOnly:
        depthEnable     = FALSE;
        depthWriteMask  = D3D11_DEPTH_WRITE_MASK_ZERO;
        break;

    }

    D3D11_DEPTH_STENCIL_DESC result = {};
    result.DepthEnable                  = depthEnable;
    result.DepthWriteMask               = depthWriteMask;
    result.DepthFunc                    = compare;
    result.StencilEnable                = FALSE;
    result.StencilReadMask              = D3D11_DEFAULT_STENCIL_READ_MASK;
    result.StencilWriteMask             = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    result.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    result.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    result.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    result.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    result.BackFace                     = result.FrontFace;

    return result;
}

//-----------------------------------------------------------------------------
//      ラスタライザーステートを取得します.
//-----------------------------------------------------------------------------
D3D11_RASTERIZER_DESC GetRasterizerState(RasterizerState type, bool multiSample)
{
    D3D11_CULL_MODE cullMode;
    D3D11_FILL_MODE fillMode;

    switch(type)
    {
    case asdx::CullNone:
    default:
        cullMode = D3D11_CULL_NONE;
        fillMode = D3D11_FILL_SOLID;
        break;

    case asdx::CullBack:
        cullMode = D3D11_CULL_BACK;
        fillMode = D3D11_FILL_SOLID;
        break;

    case asdx::CullFront:
        cullMode = D3D11_CULL_FRONT;
        fillMode = D3D11_FILL_SOLID;
        break;

    case asdx::WireFrame:
        cullMode = D3D11_CULL_NONE;
        fillMode = D3D11_FILL_WIREFRAME;
        break;
    }

    D3D11_RASTERIZER_DESC result = {};
    result.CullMode                 = cullMode;
    result.FillMode                 = fillMode;
    result.FrontCounterClockwise    = FALSE;
    result.DepthBias                = 0;
    result.DepthBiasClamp           = 0.0f;
    result.SlopeScaledDepthBias     = 0.0f;
    result.DepthClipEnable          = true;
    result.ScissorEnable            = true;
    result.MultisampleEnable        = multiSample;
    result.AntialiasedLineEnable    = (type == asdx::WireFrame) ? TRUE : FALSE;

    return result;
}

//-----------------------------------------------------------------------------
//      サンプラー設定を取得します.
//-----------------------------------------------------------------------------
D3D11_SAMPLER_DESC GetSamplerDesc(SamplerState type, uint32_t maxAnisotropy, D3D11_COMPARISON_FUNC compare)
{
    D3D11_FILTER                filter;
    D3D11_TEXTURE_ADDRESS_MODE  addressMode;

    switch(type)
    {
    case asdx::PointClamp:
        filter      = D3D11_FILTER_MIN_MAG_MIP_POINT;
        addressMode = D3D11_TEXTURE_ADDRESS_CLAMP;
        break;

    case asdx::PointWrap:
        filter      = D3D11_FILTER_MIN_MAG_MIP_POINT;
        addressMode = D3D11_TEXTURE_ADDRESS_WRAP;
        break;

    case asdx::PointMirror:
        filter      = D3D11_FILTER_MIN_MAG_MIP_POINT;
        addressMode = D3D11_TEXTURE_ADDRESS_MIRROR;
        break;

    case asdx::LinearClamp:
        filter      = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        addressMode = D3D11_TEXTURE_ADDRESS_CLAMP;
        break;

    case asdx::LinearWrap:
        filter      = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        addressMode = D3D11_TEXTURE_ADDRESS_WRAP;
        break;

    case asdx::LinearMirror:
        filter      = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        addressMode = D3D11_TEXTURE_ADDRESS_MIRROR;
        break;

    case asdx::AnisotropicClamp:
        filter      = D3D11_FILTER_ANISOTROPIC;
        addressMode = D3D11_TEXTURE_ADDRESS_CLAMP;
        break;

    case asdx::AnisotropicWrap:
        filter      = D3D11_FILTER_ANISOTROPIC;
        addressMode = D3D11_TEXTURE_ADDRESS_WRAP;
        break;

    case asdx::AnisotropicMirror:
        filter      = D3D11_FILTER_ANISOTROPIC;
        addressMode = D3D11_TEXTURE_ADDRESS_MIRROR;
        break;
    }

    D3D11_SAMPLER_DESC result = {};
    result.Filter           = filter;
    result.AddressU         = addressMode;
    result.AddressV         = addressMode;
    result.AddressW         = addressMode;
    result.MaxAnisotropy    = maxAnisotropy;
    result.MaxLOD           = FLT_MAX;
    result.ComparisonFunc   = compare;
    result.BorderColor[0]   = 1.0f;
    result.BorderColor[1]   = 1.0f;
    result.BorderColor[2]   = 1.0f;
    result.BorderColor[3]   = 1.0f;

    return result;
}

//-----------------------------------------------------------------------------
//      フルスクリーン描画用頂点シェーダを取得します.
//-----------------------------------------------------------------------------
Shader GetFullScreenVS()
{
    Shader result = {};
    result.Size     = sizeof(FullScreenVS);
    result.pBinary  = FullScreenVS;
    return result;
}

//-----------------------------------------------------------------------------
//      コピー用ピクセルシェーダを取得します.
//-----------------------------------------------------------------------------
Shader GetCopyPS()
{
    Shader result = {};
    result.Size     = sizeof(CopyPS);
    result.pBinary  = CopyPS;
    return result;
}

//-----------------------------------------------------------------------------
//      フルスクリーン描画用入力レイアウトを取得します.
//-----------------------------------------------------------------------------
InputLayoutDesc GetFullScreenLayout()
{
    InputLayoutDesc result = {};
    result.ElementCount = _countof(kFullScreenElements);
    result.pElements    = kFullScreenElements;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// GraphicsPipelineState class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GraphicsPipelineState::Init(ID3D11Device* pDevice, const GraphicsPipelineStateDesc& desc)
{
    if (pDevice == nullptr)
    { return false; }

    HRESULT hr = S_OK;

    hr = pDevice->CreateVertexShader(desc.VS.pBinary, desc.VS.Size, nullptr, m_VS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateVertexShader() Failed. errcode = 0x%x", hr);
        return false;
    }

    if (desc.DS.pBinary != nullptr && desc.DS.Size > 0)
    {
        hr = pDevice->CreateDomainShader(desc.DS.pBinary, desc.DS.Size, nullptr, m_DS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateDomainShader() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    if (desc.HS.pBinary != nullptr && desc.HS.Size > 0)
    {
        hr = pDevice->CreateHullShader(desc.HS.pBinary, desc.HS.Size, nullptr, m_HS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateHullShader() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    if (desc.PS.pBinary != nullptr && desc.PS.Size > 0)
    {
        hr = pDevice->CreatePixelShader(desc.PS.pBinary, desc.PS.Size, nullptr, m_PS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreatePixelShader() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    hr = pDevice->CreateBlendState(&desc.BlendState, m_BS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateBlendState() Failed. errcode = 0x%x", hr);
        return false;
    }

    hr = pDevice->CreateRasterizerState(&desc.RasterizerState, m_RS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateRasterizerState() Failed. errcode = 0x%x", hr);
        return false;
    }

    hr = pDevice->CreateDepthStencilState(&desc.DepthStencilState, m_DSS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateDepthStencilState() Failed. errcode = 0x%x", hr);
        return false;
    }

    hr = pDevice->CreateInputLayout(desc.InputLayout.pElements, desc.InputLayout.ElementCount, desc.VS.pBinary, desc.VS.Size, m_IL.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateInputLayout() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_Topology = desc.PrimitiveTopology;
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::Term()
{
    m_VS .Reset();
    m_DS .Reset();
    m_HS .Reset();
    m_PS .Reset();
    m_BS .Reset();
    m_RS .Reset();
    m_DSS.Reset();
    m_IL .Reset();
}

//-----------------------------------------------------------------------------
//      バインドします.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::Bind(ID3D11DeviceContext* pContext)
{
    float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    uint32_t sampleMask = D3D11_DEFAULT_SAMPLE_MASK;

    pContext->VSSetShader(m_VS.GetPtr(), nullptr, 0);
    if (m_DS.GetPtr() != nullptr)
    { pContext->DSSetShader(m_DS.GetPtr(), nullptr, 0); }
    if (m_HS.GetPtr() != nullptr)
    { pContext->HSSetShader(m_HS.GetPtr(), nullptr, 0); }
    if (m_PS.GetPtr() != nullptr)
    { pContext->PSSetShader(m_PS.GetPtr(), nullptr, 0); }

    pContext->OMSetBlendState(m_BS.GetPtr(), blendFactor, sampleMask);
    pContext->OMSetDepthStencilState(m_DSS.GetPtr(), 0);
    pContext->RSSetState(m_RS.GetPtr());
    pContext->IASetInputLayout(m_IL.GetPtr());
    pContext->IASetPrimitiveTopology(m_Topology);
}

//-----------------------------------------------------------------------------
//      バインドを解除します.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::Unbind(ID3D11DeviceContext* pContext)
{
    pContext->PSSetShader(nullptr, nullptr, 0);
    pContext->GSSetShader(nullptr, nullptr, 0);
    pContext->DSSetShader(nullptr, nullptr, 0);
    pContext->HSSetShader(nullptr, nullptr, 0);
    pContext->VSSetShader(nullptr, nullptr, 0);
}

///////////////////////////////////////////////////////////////////////////////
// ComputePipelineState class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ComputePipelineState::Init(ID3D11Device* pDevice, const Shader& computeShader)
{
    auto hr = pDevice->CreateComputeShader(computeShader.pBinary, computeShader.Size, nullptr, m_CS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateComputeShader() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ComputePipelineState::Term()
{
    m_CS.Reset();
}

//-----------------------------------------------------------------------------
//      バインドします.
//-----------------------------------------------------------------------------
void ComputePipelineState::Bind(ID3D11DeviceContext* pContext)
{
    pContext->CSSetShader(m_CS.GetPtr(), nullptr, 0);
}

//-----------------------------------------------------------------------------
//      バインドを解除します.
//-----------------------------------------------------------------------------
void ComputePipelineState::Unbind(ID3D11DeviceContext* pContext)
{
    pContext->CSSetShader(nullptr, nullptr, 0);
}


} // namespace asdx
