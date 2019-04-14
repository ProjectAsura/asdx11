//-------------------------------------------------------------------------------------------------
// File : asdxPenTab.cpp
// Desc : Pen Tablet Device.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#ifdef ASDX_ENABLE_PENTAB

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <Windows.h>
#include <WinTab.h>
#include <asdxHid.h>
#include <asdxLogger.h>


namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function pointers to Wintab functions exported from wintab32.dll. 
///////////////////////////////////////////////////////////////////////////////////////////////////
typedef UINT ( API * WTINFOA ) ( UINT, UINT, LPVOID );
typedef HCTX ( API * WTOPENA )( HWND, LPLOGCONTEXTA, BOOL );
typedef BOOL ( API * WTGETA ) ( HCTX, LPLOGCONTEXT );
typedef BOOL ( API * WTSETA ) ( HCTX, LPLOGCONTEXT );
typedef BOOL ( API * WTCLOSE ) ( HCTX );
typedef BOOL ( API * WTENABLE ) ( HCTX, BOOL );
typedef BOOL ( API * WTPACKET ) ( HCTX, UINT, LPVOID );
typedef BOOL ( API * WTOVERLAP ) ( HCTX, BOOL );
typedef BOOL ( API * WTSAVE ) ( HCTX, LPVOID );
typedef BOOL ( API * WTCONFIG ) ( HCTX, HWND );
typedef HCTX ( API * WTRESTORE ) ( HWND, LPVOID, BOOL );
typedef BOOL ( API * WTEXTSET ) ( HCTX, UINT, LPVOID );
typedef BOOL ( API * WTEXTGET ) ( HCTX, UINT, LPVOID );
typedef BOOL ( API * WTQUEUESIZESET ) ( HCTX, int );
typedef int  ( API * WTDATAPEEK ) ( HCTX, UINT, UINT, int, LPVOID, LPINT);
typedef int  ( API * WTPACKETSGET ) (HCTX, int, LPVOID);
typedef HMGR ( API * WTMGROPEN ) ( HWND, UINT );
typedef BOOL ( API * WTMGRCLOSE ) ( HMGR );
typedef HCTX ( API * WTMGRDEFCONTEXT ) ( HMGR, BOOL );
typedef HCTX ( API * WTMGRDEFCONTEXTEX ) ( HMGR, UINT, BOOL );

HINSTANCE           g_hWinTab               = nullptr;
WTINFOA             g_pWTInfoA              = nullptr;
WTOPENA             g_pWTOpenA              = nullptr;
WTGETA              g_pWTGetA               = nullptr;
WTSETA              g_pWTSetA               = nullptr;
WTCLOSE             g_pWTClose              = nullptr;
WTPACKET            g_pWTPacket             = nullptr;
WTENABLE            g_pWTEnable             = nullptr;
WTOVERLAP           g_pWTOverlap            = nullptr;
WTSAVE              g_pWTSave               = nullptr;
WTCONFIG            g_pWTConfig             = nullptr;
WTRESTORE           g_pWTRestore            = nullptr;
WTEXTSET            g_pWTExtSet             = nullptr;
WTEXTGET            g_pWTExtGet             = nullptr;
WTQUEUESIZESET      g_pWTQueueSizeSet       = nullptr;
WTDATAPEEK          g_pWTDataPeek           = nullptr;
WTPACKETSGET        g_pWTPacketsGet         = nullptr;
WTMGROPEN           g_pWTMgrOpen            = nullptr;
WTMGRCLOSE          g_pWTMgrClose           = nullptr;
WTMGRDEFCONTEXT     g_pWTMgrDefContext      = nullptr;
WTMGRDEFCONTEXTEX   g_pWTMgrDefContextEx    = nullptr;

#define GETPROCADDRESS(type, func, flags) \
    g_p##func = reinterpret_cast<type>(::GetProcAddress(g_hWinTab, #func)); \
    flags |= (g_p##func == nullptr);

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// PenTablet class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      ライブラリの初期化を行います.
//-------------------------------------------------------------------------------------------------
bool PenTablet::InitLib()
{
    g_hWinTab = LoadLibraryA( "Wintab32.dll" );
    if (g_hWinTab == nullptr)
    {
        ELOG("Error : Could not load wintab32.dll. error code = %i", GetLastError());
        return false;
    }

    bool flags = false;
    GETPROCADDRESS( WTOPENA,            WTOpenA,            flags);
    GETPROCADDRESS( WTINFOA,            WTInfoA,            flags );
    GETPROCADDRESS( WTGETA,             WTGetA,             flags );
    GETPROCADDRESS( WTSETA,             WTSetA,             flags );
    GETPROCADDRESS( WTPACKET,           WTPacket,           flags );
    GETPROCADDRESS( WTCLOSE,            WTClose,            flags );
    GETPROCADDRESS( WTENABLE,           WTEnable,           flags );
    GETPROCADDRESS( WTOVERLAP,          WTOverlap,          flags );
    GETPROCADDRESS( WTSAVE,             WTSave,             flags );
    GETPROCADDRESS( WTCONFIG,           WTConfig,           flags );
    GETPROCADDRESS( WTRESTORE,          WTRestore,          flags );
    GETPROCADDRESS( WTEXTSET,           WTExtSet,           flags );
    GETPROCADDRESS( WTEXTGET,           WTExtGet,           flags );
    GETPROCADDRESS( WTQUEUESIZESET,     WTQueueSizeSet,     flags );
    GETPROCADDRESS( WTDATAPEEK,         WTDataPeek,         flags );
    GETPROCADDRESS( WTPACKETSGET,       WTPacketsGet,       flags );
    GETPROCADDRESS( WTMGROPEN,          WTMgrOpen,          flags );
    GETPROCADDRESS( WTMGRCLOSE,         WTMgrClose,         flags );
    GETPROCADDRESS( WTMGRDEFCONTEXT,    WTMgrDefContext,    flags );
    GETPROCADDRESS( WTMGRDEFCONTEXTEX,  WTMgrDefContextEx,  flags );

    return (flags == false);
}

//-------------------------------------------------------------------------------------------------
//      ライブラリの終了処理を行います.
//-------------------------------------------------------------------------------------------------
void PenTablet::TermLib()
{
    if (g_hWinTab != nullptr)
    {
        FreeLibrary(g_hWinTab);
        g_hWinTab = nullptr;
    }

    g_hWinTab               = nullptr;
    g_pWTInfoA              = nullptr;
    g_pWTOpenA              = nullptr;
    g_pWTGetA               = nullptr;
    g_pWTSetA               = nullptr;
    g_pWTClose              = nullptr;
    g_pWTPacket             = nullptr;
    g_pWTEnable             = nullptr;
    g_pWTOverlap            = nullptr;
    g_pWTSave               = nullptr;
    g_pWTConfig             = nullptr;
    g_pWTRestore            = nullptr;
    g_pWTExtSet             = nullptr;
    g_pWTExtGet             = nullptr;
    g_pWTQueueSizeSet       = nullptr;
    g_pWTDataPeek           = nullptr;
    g_pWTPacketsGet         = nullptr;
    g_pWTMgrOpen            = nullptr;
    g_pWTMgrClose           = nullptr;
    g_pWTMgrDefContext      = nullptr;
    g_pWTMgrDefContextEx    = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
PenTablet::PenTablet()
{
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
PenTablet::~PenTablet()
{

}


} // namespace asdx

#undef GETPROCADDRESS

#endif//ASDX_ENABLE_PENTAB