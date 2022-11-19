﻿//-----------------------------------------------------------------------------
// File : asdxSky.h
// Desc : Sky Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d11.h>
#include <asdxMath.h>
#include <asdxRef.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SkyBox class
///////////////////////////////////////////////////////////////////////////////
class SkyBox
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

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    SkyBox();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~SkyBox();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(ID3D11Device* pDevice);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      描画処理を行います.
    //!
    //! @param[in]      pContext        デバイスコンテキストです.
    //! @param[in]      pSRV            キューブマップです.
    //! @param[in]      boxSize         スカイボックスのサイズです.
    //! @param[in]      view            ビュー行列です.
    //! @param[in]      proj            射影行列です.
    //-------------------------------------------------------------------------
    void Draw(
        ID3D11DeviceContext*        pContext, 
        ID3D11ShaderResourceView*   pSRV,
        ID3D11SamplerState*         pSmp,
        float                       boxSize,
        const Vector3&              cameraPos,
        const Matrix&               view,
        const Matrix&               proj);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D11Buffer>        m_pVB;      //!< 頂点バッファです.
    RefPtr<ID3D11Buffer>        m_pCB;      //!< 定数バッファです.
    RefPtr<ID3D11InputLayout>   m_pIL;      //!< 入力レイアウトです.
    RefPtr<ID3D11VertexShader>  m_pVS;      //!< 頂点シェーダです.
    RefPtr<ID3D11PixelShader>   m_pPS;      //!< ピクセルシェーダです.

    //=========================================================================
    // private methods.
    //=========================================================================
    SkyBox              (const SkyBox&) = delete;
    SkyBox& operator =  (const SkyBox&) = delete;
};


///////////////////////////////////////////////////////////////////////////////
// SkySphere class
///////////////////////////////////////////////////////////////////////////////
class SkySphere
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

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    SkySphere();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~SkySphere();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理です.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      tessellation    分割数です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D11Device* pDevice, int tessellation = 20);

    //-------------------------------------------------------------------------
    //! @brief      終了処理です.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      描画処理を行います.
    //!
    //! @param[in]      pContext        デバイスコンテキストです.
    //! @param[in]      shaderType      シェーダタイプです.
    //! @param[in]      pSRV            スフィアマップです.
    //! @param[in]      sphereSize      スカイスフィアのサイズです.
    //! @param[in]      view            ビュー行列です.
    //! @param[in]      proj            射影行列です.
    //-------------------------------------------------------------------------
    void Draw(
        ID3D11DeviceContext*        pContext,
        ID3D11ShaderResourceView*   pSRV,
        ID3D11SamplerState*         pSmp,
        float                       sphereSize,
        const Vector3&              cameraPos,
        const Matrix&               view,
        const Matrix&               proj);


    //-------------------------------------------------------------------------
    //! @brief      描画処理を行います.
    //!
    //! @param[in]      pContext        デバイスコンテキストです.
    //! @param[in]      shaderType      シェーダタイプです.
    //! @param[in]      pSRV            スフィアマップです.
    //! @param[in]      sphereSize      スカイスフィアのサイズです.
    //! @param[in]      view            ビュー行列です.
    //! @param[in]      proj            射影行列です.
    //! @param[in]      flowDir         流れの方向ベクトルです.
    //! @param[in]      flowStep        流れの加算量です.
    //-------------------------------------------------------------------------
    void DrawFlow(
        ID3D11DeviceContext*        pContext,
        ID3D11ShaderResourceView*   pSRV,
        ID3D11SamplerState*         pSmp,
        float                       sphereSize,
        const Vector3&              cameraPos,
        const Matrix&               view,
        const Matrix&               proj,
        const Vector3&              flowDir,
        float                       flowStep);

    //-------------------------------------------------------------------------
    //! @brief      フローオフセットをリセットします.
    //-------------------------------------------------------------------------
    void ResetFlow();

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D11Buffer>        m_pVB;              //!< 頂点バッファです.
    RefPtr<ID3D11Buffer>        m_pIB;              //!< インデックスバッファです.
    RefPtr<ID3D11Buffer>        m_pCB;              //!< 定数バッファです.
    RefPtr<ID3D11Buffer>        m_pCBFlow;          //!< フロー用定数バッファです.
    RefPtr<ID3D11InputLayout>   m_pIL;              //!< 入力レイアウトです.
    RefPtr<ID3D11VertexShader>  m_pVS;              //!< 頂点シェーダです.
    RefPtr<ID3D11PixelShader>   m_pPS;              //!< ピクセルシェーダです.
    RefPtr<ID3D11PixelShader>   m_pPSFlow;          //!< フロー用ピクセルシェーダです.
    uint32_t                    m_IndexCount;       //!< 頂点インデックス数です.
    float                       m_FlowOffset[2];    //!< フローオフセット.

    //=========================================================================
    // private methods.
    //=========================================================================
    SkySphere               (const SkySphere&) = delete;
    SkySphere& operator =   (const SkySphere&) = delete;
};

} // namespace asdx
