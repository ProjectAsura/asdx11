﻿//-----------------------------------------------------------------------------
// File : asdxAppHistoryMgr.h
// Desc : History Manager For Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <edit/asdxHistory.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// AppHistoryMgr class
///////////////////////////////////////////////////////////////////////////////
class AppHistoryMgr
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
    //! @brief      シングルトンインスタンスを取得します.
    //!
    //! @return     シングルトンインスタンスを返却します.
    //-------------------------------------------------------------------------
    static AppHistoryMgr& GetInstance();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      capacity        登録可能な履歴数.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(int capacity);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      履歴を登録します.
    //!
    //! @param[in]      item        動的メモリ確保済み履歴インスタンス.
    //! @param[in]      redo        登録時にRedo()を実行する場合は true.
    //-------------------------------------------------------------------------
    void Add(asdx::IHistory* item, bool redo = true);

    //-------------------------------------------------------------------------
    //! @brief      履歴をクリアします.
    //-------------------------------------------------------------------------
    void Clear();

    //-------------------------------------------------------------------------
    //! @brief      やり直します.
    //-------------------------------------------------------------------------
    void Redo();

    //-------------------------------------------------------------------------
    //! @brief      元に戻します.
    //-------------------------------------------------------------------------
    void Undo();

    //-------------------------------------------------------------------------
    //! @brief      初期化済みかどうか?
    //-------------------------------------------------------------------------
    bool IsInit() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    static AppHistoryMgr    s_Instance;
    asdx::HistoryMgr        m_Manager;

    //=========================================================================
    // private methods.
    //=========================================================================
    AppHistoryMgr();
    ~AppHistoryMgr();

    AppHistoryMgr               (const AppHistoryMgr&) = delete;
    AppHistoryMgr& operator =   (const AppHistoryMgr&) = delete;
};

} // namespace asdx
