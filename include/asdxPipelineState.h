//-----------------------------------------------------------------------------
// File : asdxPipelineState.h
// Desc : Pipeline State Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <d3d11.h>
#include <asdxRef.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// BlendState enum
///////////////////////////////////////////////////////////////////////////////
enum BlendState
{
    Opaque = 0,             //!< 不透明
    AlphaBlend,             //!< アルファブレンド.
    Additive,               //!< 加算.
    Subtract,               //!< 減算.
    Premultiplied,          //!< 事前乗算済みアルファブレンド.
    Multiply,               //!< 乗算.
    Screen,                 //!< スクリーン.
    NumBlendType,
};

///////////////////////////////////////////////////////////////////////////////
// DepthState enum
///////////////////////////////////////////////////////////////////////////////
enum DepthState
{
    Default = 0,        //!< 深度テストあり, 深度書き込みあり.
    ReadOnly,           //!< 深度テストあり, 深度書き込みなし.
    WriteOnly,          //!< 深度テストなし, 深度書き込みあり.
    None,               //!< 深度テストなし, 深度書き込みなし.
    NumDepthType,
};

///////////////////////////////////////////////////////////////////////////////
// RasterizerState enum
///////////////////////////////////////////////////////////////////////////////
enum RasterizerState
{
    CullNone = 0,               //!< カリングなし.
    CullBack,
    CullFront,
    WireFrame,                  //!< ワイヤーフレーム.
    NumRasterizerType
};

///////////////////////////////////////////////////////////////////////////////
// SamplerState enum
///////////////////////////////////////////////////////////////////////////////
enum SamplerState
{
    PointWrap = 0,       //!< ポイントサンプリング・繰り返し
    PointClamp,          //!< ポイントサンプリング・クランプ.
    PointMirror,         //!< ポイントサンプリング・ミラー.
    LinearWrap,          //!< 線形補間・繰り返し.
    LinearClamp,         //!< 線形補間・クランプ.
    LinearMirror,        //!< 線形補間・ミラー.
    AnisotropicWrap,     //!< 異方性補間・繰り返し.
    AnisotropicClamp,    //!< 異方性補間・クランプ.
    AnisotropicMirror,   //!< 異方性補間・ミラー.
    NumSamplerType,
};

///////////////////////////////////////////////////////////////////////////////
// Shader structure
///////////////////////////////////////////////////////////////////////////////
struct Shader
{
    const void* pBinary;
    size_t      Size;
};

///////////////////////////////////////////////////////////////////////////////
// InputLayoutDesc structure
///////////////////////////////////////////////////////////////////////////////
struct InputLayoutDesc
{
    uint32_t                        ElementCount;
    const D3D11_INPUT_ELEMENT_DESC* pElements;
};

//-----------------------------------------------------------------------------
//! @brief      ブレンド設定を取得します.
//-----------------------------------------------------------------------------
D3D11_BLEND_DESC GetBlendDesc(BlendState type);

//-----------------------------------------------------------------------------
//! @brief      深度ステンシル設定を取得します.
//-----------------------------------------------------------------------------
D3D11_DEPTH_STENCIL_DESC GetDepthStencilDesc(DepthState type, D3D11_COMPARISON_FUNC compare = D3D11_COMPARISON_LESS_EQUAL);

//-----------------------------------------------------------------------------
//! @brief      ラスタライザー設定を取得します.
//-----------------------------------------------------------------------------
D3D11_RASTERIZER_DESC GetRasterizerDesc(RasterizerState type, bool multiSample = false);

//-----------------------------------------------------------------------------
//! @brief      サンプラー設定を取得します.
//-----------------------------------------------------------------------------
D3D11_SAMPLER_DESC GetSamplerDesc(SamplerState type, uint32_t maxAnisotropy = 16, D3D11_COMPARISON_FUNC compare = D3D11_COMPARISON_NEVER);

//-----------------------------------------------------------------------------
//! @brief      フルスクリーン描画用頂点シェーダを取得します.
//-----------------------------------------------------------------------------
Shader GetFullScreenVS();

//-----------------------------------------------------------------------------
//! @brief      コピー用ピクセルシェーダを取得します.
//-----------------------------------------------------------------------------
Shader GetCopyPS();

//-----------------------------------------------------------------------------
//! @brief      フルスクリーン描画用入力レイアウトを取得します.
//-----------------------------------------------------------------------------
InputLayoutDesc GetFullScreenLayout();


///////////////////////////////////////////////////////////////////////////////
// GraphicsPipelineState structure
///////////////////////////////////////////////////////////////////////////////
struct GraphicsPipelineStateDesc
{
    Shader                      VS                  = {};
    Shader                      DS                  = {};
    Shader                      HS                  = {};
    Shader                      PS                  = {};
    D3D11_BLEND_DESC            BlendState          = {};
    D3D11_RASTERIZER_DESC       RasterizerState     = {};
    D3D11_DEPTH_STENCIL_DESC    DepthStencilState   = {};
    InputLayoutDesc             InputLayout         = {};
    D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology   = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

///////////////////////////////////////////////////////////////////////////////
// GraphicsPipelineState class
///////////////////////////////////////////////////////////////////////////////
class GraphicsPipelineState
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    bool Init(ID3D11Device* pDevice, const GraphicsPipelineStateDesc& desc);
    void Term();
    void Bind(ID3D11DeviceContext* pContext);
    void Unbind(ID3D11DeviceContext* pContext);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D11VertexShader>      m_VS;
    RefPtr<ID3D11DomainShader>      m_DS;
    RefPtr<ID3D11HullShader>        m_HS;
    RefPtr<ID3D11PixelShader>       m_PS;
    RefPtr<ID3D11BlendState>        m_BS;
    RefPtr<ID3D11RasterizerState>   m_RS;
    RefPtr<ID3D11DepthStencilState> m_DSS;
    RefPtr<ID3D11InputLayout>       m_IL;
    D3D11_PRIMITIVE_TOPOLOGY        m_Topology  = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// ComputePipelineState class
///////////////////////////////////////////////////////////////////////////////
class ComputePipelineState
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    bool Init(ID3D11Device* pDevice, const Shader& computeShader);
    void Term();
    void Bind(ID3D11DeviceContext* pContext);
    void Unbind(ID3D11DeviceContext* pContext);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D11ComputeShader> m_CS;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
