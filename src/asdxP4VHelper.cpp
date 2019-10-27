//-----------------------------------------------------------------------------
// File : asdxP4VHelper.cpp
// Desc : Perforce Helix P4V Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxP4VHelper.h>
#include <asdxMisc.h>
#include <Windows.h>


namespace asdx {

//-----------------------------------------------------------------------------
//      デフォルトのチェンジリストに対象ファイルを追加します.
//-----------------------------------------------------------------------------
bool AddP4V(const char* path, const char* changeList)
{
    char fullPath[MAX_PATH];
    char cmd[2048];

    GetFullPathNameA(path, MAX_PATH, fullPath, nullptr);
    sprintf_s(cmd, "p4 add -d -c %s %s", changeList, fullPath);

    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      デフォルトのチェンジリストに対象ファイルをチェックアウトします.
//-----------------------------------------------------------------------------
bool CheckoutP4V(const char* path, const char* changeList)
{
    char fullPath[MAX_PATH];
    char cmd[2048];

    GetFullPathNameA(path, MAX_PATH, fullPath, nullptr);
    sprintf_s(cmd, "p4 edit -c %s %s", changeList, fullPath);

    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      対象ファイルの変更を元に戻します.
//-----------------------------------------------------------------------------
bool RevertP4V(const char* path, const char* changeList)
{
    char fullPath[MAX_PATH];
    char cmd[2048];

    GetFullPathNameA(path, MAX_PATH, fullPath, nullptr);
    sprintf_s(cmd, "p4 revert -a -c %s %s", changeList, fullPath);

    return RunProcess(cmd);
}

} // namespace asdx
