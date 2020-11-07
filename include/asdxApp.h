﻿//-----------------------------------------------------------------------------
// File : asdxApp.h
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <mutex>
#include <vector>
#include <string>

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_6.h>

#include <asdxRef.h>
#include <asdxTarget.h>
#include <asdxTimer.h>
#include <asdxHid.h>

#if defined(ASDX_ENABLE_D2D)
#include <d2d1_1.h>
#include <dwrite.h>
#endif//defined(ASDX_ENABLE_D2D)

#if defined(DEBUG) || defined(_DEBUG)
#include <DXGIDebug.h>
#endif//defiend(DEBUG) || defined(_DEBUG)

//-----------------------------------------------------------------------------
// Linker
//-----------------------------------------------------------------------------
#ifdef ASDX_AUTO_LINK
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "comctl32.lib" )

#if defined(ASDX_ENABLE_D2D)
#pragma comment( lib, "d2d1.lib" )
#pragma comment( lib, "dwrite.lib" )
#endif//defined(ASDX_ENABLE_D2D)

#endif//ASDX_AUTO_LINK


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// COLOR_SPACE enum
///////////////////////////////////////////////////////////////////////////////
enum COLOR_SPACE
{
    COLOR_SPACE_NONE,           // デフォルト.
    COLOR_SPACE_SRGB,           // SRGB (ガンマ2.2)
    COLOR_SPACE_BT709,          // ITU-R BT.709 (ガンマ2.4)
    COLOR_SPACE_BT2100_PQ,      // ITU-R BT.2100 Perceptual Quantizer
    COLOR_SPACE_BT2100_HLG,     // ITU-R BT.2100 Hybrid Log Gamma
};

///////////////////////////////////////////////////////////////////////////////
// MouseEventArgs structure
///////////////////////////////////////////////////////////////////////////////
struct MouseEventArgs
{
    int     X;                  //!< カーソルのX座標です.
    int     Y;                  //!< カーソルのY座標です.
    int     WheelDelta;         //!< マウスホイールの移動方向です.
    bool    IsLeftButtonDown;   //!< 左ボタンが押されたどうかを示すフラグです.
    bool    IsRightButtonDown;  //!< 右ボタンが押されたどうかを示すフラグです.
    bool    IsMiddleButtonDown; //!< 中ボタンが押されたかどうかを示すフラグです.
    bool    IsSideButton1Down;  //!< X1ボタンが押されたかどうかを示すフラグです.
    bool    IsSideButton2Down;  //!< X2ボタンが押されたかどうかを示すフラグです.

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    MouseEventArgs()
    : X                 ( 0 )
    , Y                 ( 0 )
    , WheelDelta        ( 0 )
    , IsLeftButtonDown  ( false )
    , IsRightButtonDown ( false)
    , IsMiddleButtonDown( false )
    , IsSideButton1Down ( false )
    , IsSideButton2Down ( false )
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////
// KeyEventArgs structure
///////////////////////////////////////////////////////////////////////////////
struct KeyEventArgs
{
    uint32_t    KeyCode;        //!< キーコードです.
    bool        IsKeyDown;      //!< キーが押されたかどうかを示すフラグです.
    bool        IsAltDown;      //!< ALTキーが押されたかどうかを示すフラグです.

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    KeyEventArgs()
    : KeyCode   ( 0 )
    , IsKeyDown ( false )
    , IsAltDown ( false )
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////
// ResizeEventArgs structure
///////////////////////////////////////////////////////////////////////////////
struct ResizeEventArgs
{
    uint32_t    Width;          //!< 画面の横幅です.
    uint32_t    Height;         //!< 画面の縦幅です.
    float       AspectRatio;    //!< 画面のアスペクト比です.

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    ResizeEventArgs()
    : Width      ( 0 )
    , Height     ( 0 )
    , AspectRatio( 0.0f )
    { /* DO_NOTHING */ }
};


///////////////////////////////////////////////////////////////////////////////
// FrameEventArgs struture
///////////////////////////////////////////////////////////////////////////////
struct FrameEventArgs
{
    ID3D11DeviceContext*    pDeviceContext; //!< デバイスコンテキストです.
    double                  Time;           //!< アプリケーション開始からの相対時間です.
    double                  ElapsedTime;    //!< 前のフレームからの経過時間(秒)です.
    float                   FPS;            //!< １秒当たりフレーム更新回数です.
    bool                    IsStopDraw;     //!< 描画停止フラグです.

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    FrameEventArgs()
    : pDeviceContext( nullptr )
    , Time          ( 0 )
    , ElapsedTime   ( 0 )
    , FPS           ( 0.0f )
    , IsStopDraw    ( false )
    { /* DO_NOTHING */ }
};

///////////////////////////////////////////////////////////////////////////////
// Application class
///////////////////////////////////////////////////////////////////////////////
class Application
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
    Application();

    //-------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //!
    //! @param [in]     title       タイトル名.
    //! @param [in]     width       画面の横幅.
    //! @param [in]     height      画面の縦幅.
    //! @param [in]     hIcon       アイコンハンドル.
    //! @param [in]     hMenu       メニューハンドル.
    //! @param [in]     hAccel      アクセレレータハンドル.
    //-------------------------------------------------------------------------
    Application( LPCWSTR title, uint32_t width, uint32_t height, HICON hIcon, HMENU hMenu, HACCEL hAccel );

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    virtual ~Application();

    //-------------------------------------------------------------------------
    //! @brief      アプリケーションを実行します.
    //-------------------------------------------------------------------------
    void Run();

    //-------------------------------------------------------------------------
    //! @brief      フォーカスを持つかどうか判定します.
    //!
    //! @retval true    フォーカスを持ちます.
    //! @retval false   フォーカスを持ちません.
    //-------------------------------------------------------------------------
    bool HasFocus() const;

protected:
    //=========================================================================
    // protected variables
    //=========================================================================
    HINSTANCE                       m_hInst;                //!< インスタンスハンドルです.
    HWND                            m_hWnd;                 //!< ウィンドウハンドルです.
    ID3D11Device*                   m_pDevice;              //!< デバイスです.
    ID3D11DeviceContext*            m_pDeviceContext;       //!< デバイスコンテキストです.
    uint32_t                        m_MultiSampleCount;     //!< マルチサンプリングのカウント数です.
    uint32_t                        m_MultiSampleQuality;   //!< マルチサンプリングの品質値です.
    uint32_t                        m_SwapChainCount;       //!< スワップチェインのバッファ数です.
    DXGI_FORMAT                     m_SwapChainFormat;      //!< スワップチェインのバッファフォーマットです.
    DXGI_FORMAT                     m_DepthStencilFormat;   //!< 深度ステンシルバッファのフォーマットです.
    RefPtr<IDXGISwapChain>          m_pSwapChain;           //!< スワップチェインです.
    RefPtr<IDXGISwapChain4>         m_pSwapChain4;          //!< スワップチェイン4です.
    ColorTarget2D                   m_ColorTarget2D;        //!< 描画ターゲットです.
    DepthTarget2D                   m_DepthTarget2D;        //!< 深度ステンシルバッファです.
    float                           m_ClearColor[ 4 ];      //!< 背景のクリアカラーです.
    uint32_t                        m_Width;                //!< 画面の横幅です.
    uint32_t                        m_Height;               //!< 画面の縦幅です.
    float                           m_AspectRatio;          //!< 画面のアスペクト比です.
    LPCWSTR                         m_Title;                //!< アプリケーションのタイトル名です.
    Timer                           m_Timer;                //!< タイマーです.
    D3D11_VIEWPORT                  m_Viewport;             //!< ビューポートです.
    D3D11_RECT                      m_ScissorRect;          //!< シザー矩形です.
    HICON                           m_hIcon;                //!< アイコンハンドルです.
    HMENU                           m_hMenu;                //!< メニューハンドルです.
    HACCEL                          m_hAccel;               //!< アクセレレータハンドルです.

#if ASDX_IS_DEBUG
    RefPtr<ID3D11Debug>             m_pD3D11Debug;          //!< デバッグオブジェクトです.
#endif//ASDX_IS_DEBUG

#if defined(ASDX_ENABLE_D2D)
    RefPtr<ID2D1Factory1>           m_pFactory2D;           //!< D2D1ファクトリーです.
    RefPtr<ID2D1Device>             m_pDevice2D;            //!< D2D1デバイスです.
    RefPtr<ID2D1DeviceContext>      m_pDeviceContext2D;     //!< D2D1デバイスコンテキストです.
    RefPtr<ID2D1Bitmap1>            m_pBitmap2D;            //!< D2D1ビットマップです.
    RefPtr<IDWriteFactory>          m_pFactoryDW;           //!< DirectWriteファクトリーです.
    RefPtr<IDWriteTextFormat>       m_pDefaultTextFormat;   //!< デフォルトテキストフォーマットです.
    RefPtr<ID2D1SolidColorBrush>    m_pDefaultBrush;
#endif//defined(ASDX_ENABLE_D2D)

    //=========================================================================
    // protected methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      初期化時に実行する処理です.
    //!
    //! @note       派生クラスにて実装を行います.
    //-------------------------------------------------------------------------
    virtual bool OnInit       ();

    //-------------------------------------------------------------------------
    //! @brief      終了時に実行する処理です.
    //!
    //! @note       派生クラスにて実装を行います.
    //-------------------------------------------------------------------------
    virtual void OnTerm       ();

    //-------------------------------------------------------------------------
    //! @brief      フレーム遷移時に実行する処理です.
    //!
    //! @param [in]     param       フレームインベントパラメータです.
    //! @note       派生クラスにて実装を行います.
    //-------------------------------------------------------------------------
    virtual void OnFrameMove  ( FrameEventArgs& param );

    //-------------------------------------------------------------------------
    //! @brief      フレーム描画時に実行する処理です.
    //!
    //! @param [in]     param       フレームインベントパラメータです.
    //! @note       派生クラスにて実装を行います.
    //-------------------------------------------------------------------------
    virtual void OnFrameRender( FrameEventArgs& param );

    //-------------------------------------------------------------------------
    //! @brief      リサイズ時に実行する処理です.
    //!
    //! @param [in]     param       リサイズイベントパラメータです.
    //! @note       派生クラスにて実装を行います.
    //-------------------------------------------------------------------------
    virtual void OnResize     ( const ResizeEventArgs& param );

    //-------------------------------------------------------------------------
    //! @brief      キーイベント通知時に実行する処理です.
    //!
    //! @param [in]     param       キーイベントパラメータです.
    //! @note       派生クラスにて実装を行います.
    //-------------------------------------------------------------------------
    virtual void OnKey        ( const KeyEventArgs&    param );

    //-------------------------------------------------------------------------
    //! @brief      マウスイベント通知時に実行する処理です.
    //!
    //! @param [in]     param       マウスイベントパラメータです.
    //! @note       派生クラスにて実装を行います.
    //-------------------------------------------------------------------------
    virtual void OnMouse      ( const MouseEventArgs&  param );

    //-------------------------------------------------------------------------
    //! @brief      タイピング時の処理です.
    //-------------------------------------------------------------------------
    virtual void OnTyping     ( uint32_t keyCode );

    //-------------------------------------------------------------------------
    //! @brief      ウィンドウへのドラッグアンドドロップされたと時に実行する処理です.
    //!
    //! @param[in]      dropFiles     ドラッグアンドドロップされたファイル名です.
    //-------------------------------------------------------------------------
    virtual void OnDrop        ( const std::vector<std::string>& dropFiles );

    //-------------------------------------------------------------------------
    //! @brief      メッセージプロシージャの処理です.
    //!
    //! @param[in]      hWnd        ウィンドウハンドル.
    //! @param[in]      msg         メッセージです.
    //! @param[in]      wp          メッセージの追加情報.
    //! @param[in]      lp          メッセージの追加情報.
    //-------------------------------------------------------------------------
    virtual void OnMsgProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp );

    //-------------------------------------------------------------------------
    //! @brief      描画停止フラグを設定します.
    //!
    //! @param [in]     isStopRendering     描画を停止するかどうか.停止する場合はtrueを指定します.
    //-------------------------------------------------------------------------
    void SetStopRendering( bool isStopRendering );

    //-------------------------------------------------------------------------
    //! @brief      描画停止フラグを取得します.
    //!
    //! @retval true    描画処理を呼び出しません.
    //! @retval false   描画処理を呼び出します.
    //-------------------------------------------------------------------------
    bool IsStopRendering();

    //-------------------------------------------------------------------------
    //! @brief      スタンバイモードかどうかチェックします.
    //!
    //! @retval true    スタンバイモードです.
    //! @retval false   非スタンバイモードです.
    //-------------------------------------------------------------------------
    bool IsStandByMode() const;

    //-------------------------------------------------------------------------
    //! @brief      フレームカウントを取得します.
    //!
    //! @return     フレームカウントを返却します.
    //-------------------------------------------------------------------------
    DWORD GetFrameCount();

    //-------------------------------------------------------------------------
    //! @brief      FPSを取得します.
    //!
    //! @return     0.5秒ごとに更新されるFPSを返却します.
    //! @note       各フレームにおけるFPSを取得する場合は FrameEventArgs から取得します.
    //-------------------------------------------------------------------------
    float GetFPS();

    //-------------------------------------------------------------------------
    //! @brief      コマンドを実行して，画面に表示します.
    //!
    //! @param [in]     syncInterval        垂直同期の間隔です.
    //-------------------------------------------------------------------------
    void Present( uint32_t syncInterval );

    //-------------------------------------------------------------------------
    //! @brief      HDR出力をサポートしているかどうかチェックします.
    //!
    //! @return     HDR出力をサポートしていれば true を返却します.
    //-------------------------------------------------------------------------
    bool IsSupportHDR() const;

    //-------------------------------------------------------------------------
    //! @brief      ディスプレイ設定を取得します.
    //!
    //! @return     ディスプレイ設定を返却します.
    //-------------------------------------------------------------------------
    DXGI_OUTPUT_DESC1 GetDisplayDesc() const;

    //-------------------------------------------------------------------------
    //! @brief      出力色空間を設定します
    //!
    //! @param[in]      value       設定する色空間.
    //! @retval true    設定に成功.
    //! @retval false   設定に失敗.
    //-------------------------------------------------------------------------
    bool SetColorSpace(COLOR_SPACE value);

    //-------------------------------------------------------------------------
    //! @brief      ディスプレイのリフレッシュレートを取得します.
    //!
    //! @param[out]     result      リフレッシュレート.
    //! @retval true    取得に成功.
    //! @retval false   取得に失敗.
    //-------------------------------------------------------------------------
    bool GetDisplayRefreshRate(DXGI_RATIONAL& result) const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    bool                m_IsStopRendering;      //!< 描画を停止するかどうかのフラグ. 停止する場合はtrueを指定.
    bool                m_IsStandbyMode;        //!< スタンバイモードかどうかを示すフラグです.
    DWORD               m_FrameCount;           //!< フレームカウントです.
    float               m_FPS;                  //!< FPS(1秒あたりのフレーム描画回数)です.
    double              m_LatestUpdateTime;     //!< 最後の更新時間です.
    std::mutex          m_Mutex;                //!< ミューテックスです.
    DXGI_OUTPUT_DESC1   m_DisplayDesc;          //!< 出力先の設定です.

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      アプリケーションの初期化処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //! @note       このメソッドは内部処理で，InitWnd(), InitD3D()を呼び出します.
    //-------------------------------------------------------------------------
    bool InitApp();

    //-------------------------------------------------------------------------
    //! @brief      アプリケーションの終了処理です.
    //!
    //! @note       このメソッドは内部処理で, TermD3D(), TermWnd()を呼び出します.
    //-------------------------------------------------------------------------
    void TermApp();

    //-------------------------------------------------------------------------
    //! @brief      ウィンドウの初期化処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool InitWnd();

    //-------------------------------------------------------------------------
    //! @brief      ウィンドウの終了処理です.
    //-------------------------------------------------------------------------
    void TermWnd();

    //-------------------------------------------------------------------------
    //! @brief      Direct3Dの初期化処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool InitD3D();

    //-------------------------------------------------------------------------
    //! @brief      Direct3Dの終了処理です.
    //-------------------------------------------------------------------------
    void TermD3D();

    //-------------------------------------------------------------------------
    //! @brief      Direct2Dの初期化処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool InitD2D();

    //-------------------------------------------------------------------------
    //! @brief      Direct2Dの終了処理です.
    //-------------------------------------------------------------------------
    void TermD2D();

    //-------------------------------------------------------------------------
    //! @brief      メインループ処理です.
    //-------------------------------------------------------------------------
    void MainLoop();

    //-------------------------------------------------------------------------
    //! @brief      キーイベントを処理します.
    //!
    //! @param[in]      param       キーイベント引数です.
    //! @note       このメソッドは内部処理で，OnKey()を呼び出します.
    //!             また，このメソッドはウィンドウプロシージャからのアクセス専用メソッドですので,
    //!             アプリケーション側で呼び出しを行わないでください.
    //-------------------------------------------------------------------------
    void KeyEvent   ( const KeyEventArgs&    param );

    //-------------------------------------------------------------------------
    //! @brief      リサイズイベントを処理します.
    //!
    //! @param[in]      param       リサイズイベント引数です.
    //! @note       このメソッドは内部処理で, OnResize()を呼び出します.
    //!             また，このメソッドはウィンドウプロシージャからのアクセス専用メソッドですので,
    //!             アプリケーション側で呼び出しを行わないでください.
    //-------------------------------------------------------------------------
    void ResizeEvent( const ResizeEventArgs& param );

    //-------------------------------------------------------------------------
    //! @brief      マウスイベントを処理します.
    //!
    //! @param[in]      param       マウスイベント引数です.
    //! @note       このメソッドは内部処理で, OnMouse()を呼び出します.
    //!             また，このメソッドはウィンドウプロシージャからのアクセス専用メソッドですので,
    //!             アプリケーション側で呼び出しを行わないでください.
    //-------------------------------------------------------------------------
    void MouseEvent( const MouseEventArgs&  param );

    //-------------------------------------------------------------------------
    //! @brief      ドロップイベントを処理します.
    //!
    //! @param[in]      dropFiles        ドロップされたファイルパスです.
    //-------------------------------------------------------------------------
    void DropEvent( const std::vector<std::string>& dropFiles );

    //-------------------------------------------------------------------------
    //! @brief      HDR出力をサポートしているかどうかチェックします.
    //-------------------------------------------------------------------------
    void CheckSupportHDR();

    //-------------------------------------------------------------------------
    //! @brief      ウィンドウプロシージャです.
    //!
    //! @param [in]     hWnd        ウィンドウハンドル.
    //! @param [in]     uMsg        メッセージ.
    //! @param [in]     wp          メッセージの追加情報.
    //! @param [in]     lp          メッセージの追加情報.
    //-------------------------------------------------------------------------
    static LRESULT CALLBACK MsgProc( HWND hWnd, uint32_t uMsg, WPARAM wp, LPARAM lp );

    Application             (const Application&) = delete;
    Application& operator = (const Application&) = delete;
};

} // namespace asdx
