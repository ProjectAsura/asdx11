﻿//-----------------------------------------------------------------------------
// File : asdxP4VHelper.
// Desc : Perforce Helix P4V Helper.
// Copyright(c) Project Asura. All right reserved. 
//-----------------------------------------------------------------------------
#pragma once


namespace asdx {

//-----------------------------------------------------------------------------
//! @brief      チェンジリストに対象ファイルを追加します.
//!
//! @param[in]      path            対象ファイルのパス.
//! @param[in]      changeList      チェンジリスト名.
//! @return     プロセス起動の結果を返却します.
//-----------------------------------------------------------------------------
bool P4Add(const char* path, const char* changeList="default");

//-----------------------------------------------------------------------------
//! @brief      チェンジリストに対象ファイルをチェックアウトします.
//!
//! @param[in]      path            対象ファイルのパス.
//! @param[in]      changeList      チェンジリスト名.
//! @return     プロセス起動の結果を返却します.
//-----------------------------------------------------------------------------
bool P4Checkout(const char* path, const char* changeList="default");

//-----------------------------------------------------------------------------
//! @brief      対象ファイルの変更を元に戻します.
//!
//! @param[in]      path            対象ファイルのパス.
//! @param[in]      onlyUnchanged   変更が無かったファイルだけ戻す場合は true.
//! @param[in]      changeList      チェンジリスト名.
//! @return     プロセス起動の結果を返却します.
//-----------------------------------------------------------------------------
bool P4Revert(const char* path, bool onlyUnchanged = false, const char* changeList="default");

//-----------------------------------------------------------------------------
//! @brief      対象ファイルを削除目的でマークします.
//!
//! @param[in]      path            対象ファイルのパス.
//! @param[in]      changeList      チェンジリスト名.
//! @return     プロセス起動の結果を返却します.
//-----------------------------------------------------------------------------
bool P4Delete(const char* path, const char* changeList="default");

//-----------------------------------------------------------------------------
//! @brief      ファイルを追加、削除、および/または編集目的で作業状態にし、Perforce外部での変更内容とワークスペースとを一致させます.
//!
//! @param[in]      path            対象ファイルのパス.
//! @param[in]      changeList      チェンジリスト名.
//! @return     プロセス起動の結果を返却します.
//-----------------------------------------------------------------------------
bool P4Reconcile(const char* path, const char* changeList="default");


} // namespace asdx
