﻿//-------------------------------------------------------------------------------------------------
// File : asdxStructuredBuffer.h
// Desc : Structured Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3d11.h>
#include <asdxRef.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// StructuredBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////
class StructuredBuffer
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    StructuredBuffer();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~StructuredBuffer();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      size            バッファサイズです.
    //! @paran[in]      stride          ストライドです.
    //! @param[in]      pInitData       初期化データです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool Init(ID3D11Device* pDevice, uint32_t sie, uint32_t stride, const void* pInitData);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理です.
    //---------------------------------------------------------------------------------------------
    void Term();

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファを取得します.
    //!
    //! @return     バッファを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11Buffer* const GetBuffer() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを取得します.
    //!
    //! @return     シェーダリソースビューを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11ShaderResourceView* const GetShaderResource() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      アロー演算子です.
    //---------------------------------------------------------------------------------------------
    ID3D11Buffer* const operator -> () const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    RefPtr<ID3D11Buffer>                m_Buffer;   //!< バッファです.
    RefPtr<ID3D11ShaderResourceView>    m_SRV;      //!< シェーダリソースビューです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace asdx