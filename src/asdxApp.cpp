//-----------------------------------------------------------------------------
// File : asdxApp.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <list>
#include <cassert>
#include <asdxApp.h>
#include <asdxMath.h>
#include <asdxLogger.h>
#include <asdxMisc.h>
#include <asdxSound.h>
#include <asdxDeviceContext.h>


namespace /* anonymous */ {

//-----------------------------------------------------------------------------
//      領域の交差を計算します.
//-----------------------------------------------------------------------------
inline int ComputeIntersectionArea
(
    int ax1, int ay1,
    int ax2, int ay2,
    int bx1, int by1,
    int bx2, int by2
)
{
    return asdx::Max(0, asdx::Min(ax2, bx2) - asdx::Max(ax1, bx1))
         * asdx::Max(0, asdx::Min(ay2, by2) - asdx::Max(ay1, by1));
}

//-----------------------------------------------------------------------------
//      nullptrかどうかを考慮してdeleteします.
//-----------------------------------------------------------------------------
template<typename T>
void SafeDelete(T*& ptr)
{
    if (ptr != nullptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      nullptrかどうかを考慮してdelete[]します.
//-----------------------------------------------------------------------------
template<typename T>
void SafeDeleteArray(T*& ptr)
{
    if (ptr != nullptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      nullptrかどうかを考慮して解放処理を行います.
//-----------------------------------------------------------------------------
template<typename T>
void SafeRelease(T*& ptr)
{
    if (ptr != nullptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      色度を変換した値を取得します.
//-----------------------------------------------------------------------------
inline UINT GetCoord(float value)
{ return static_cast<UINT>(value * 50000.0f); }

//-----------------------------------------------------------------------------
//      輝度を変換した値を取得します.
//-----------------------------------------------------------------------------
inline UINT GetLuma(float value)
{ return static_cast<UINT>(value * 10000.0f); }

//-----------------------------------------------------------------------------
//      DXGI_SWAP_EFFECT_FLIP_XXXX が使用可能かチェックします.
//-----------------------------------------------------------------------------
inline bool IsFlipEffect(DXGI_FORMAT format)
{
    switch(format)
    {
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return true;

    default:
        break;
    }

    return false;
}

} // namespace /* anonymous */


namespace asdx  {

// ウィンドウクラス名です.
#ifndef ASDX_WND_CLASSNAME
#define ASDX_WND_CLASSNAME      TEXT("asdxWindowClass")
#endif//ASDX_WND_CLAASNAME


///////////////////////////////////////////////////////////////////////////////////////////////////
// Application class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Application::Application()
: Application(L"asdxApplication", 960, 540, nullptr, nullptr, nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
Application::Application( LPCWSTR title, UINT width, UINT height, HICON hIcon, HMENU hMenu, HACCEL hAccel )
: m_hInst               ( nullptr )
, m_hWnd                ( nullptr )
, m_pDevice             ( nullptr )
, m_pDeviceContext      ( nullptr )
, m_MultiSampleCount    ( 4 )
, m_MultiSampleQuality  ( 0 )
, m_SwapChainCount      ( 2 )
, m_SwapChainFormat     ( DXGI_FORMAT_B8G8R8A8_UNORM_SRGB )
, m_DepthStencilFormat  ( DXGI_FORMAT_D24_UNORM_S8_UINT )
, m_ColorTarget2D       ()
, m_DepthTarget2D       ()
, m_Width               ( width )
, m_Height              ( height )
, m_AspectRatio         ( (float)width/(float)height )
, m_Title               ( title )
, m_Timer               ()
, m_FrameCount          ( 0 )
, m_FPS                 ( 0.0f )
, m_LatestUpdateTime    ( 0.0f )
, m_IsStandbyMode       ( false )
, m_hIcon               ( hIcon )
, m_hMenu               ( hMenu )
, m_hAccel              ( hAccel )
#if ASDX_IS_DEBUG
, m_pD3D11Debug         ( nullptr )
#endif//ASDX_IS_DEBUG
#if defined(ASDX_ENABLE_D2D)
, m_pFactory2D          ( nullptr )
, m_pDevice2D           ( nullptr )
, m_pDeviceContext2D    ( nullptr )
, m_pFactoryDW          ( nullptr )
, m_pDefaultBrush       ( nullptr )
#endif//defined(ASDX_ENABLE_D2D)
{
    // タイマーを開始します.
    m_Timer.Start();

    // 開始時刻を取得.
    m_Timer.End();
    m_LatestUpdateTime = m_Timer.GetElapsedSec();

    m_ClearColor[0] = 0.392156899f;
    m_ClearColor[1] = 0.584313750f;
    m_ClearColor[2] = 0.929411829f;
    m_ClearColor[3] = 1.000000000f;
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Application::~Application()
{ TermApp(); }

//-----------------------------------------------------------------------------
//      フレームカウントを取得します.
//-----------------------------------------------------------------------------
DWORD Application::GetFrameCount()
{ return m_FrameCount; }

//-----------------------------------------------------------------------------
//      FPSを取得します.
//-----------------------------------------------------------------------------
FLOAT Application::GetFPS()
{ return m_FPS; }

//-----------------------------------------------------------------------------
//      アプリケーションを初期化します.
//-----------------------------------------------------------------------------
bool Application::InitApp()
{
    // タイマー分解能を上げる.
    timeBeginPeriod(1);

    // COMライブラリの初期化.
    HRESULT hr = CoInitialize( nullptr );
    if ( FAILED(hr) )
    {
        DLOG( "Error : Com Library Initialize Failed." );
        return false;
    }

    // COMライブラリのセキュリティレベルを設定.
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    // セキュリティ設定の結果をチェック.
    if ( FAILED(hr) )
    {
        DLOG( "Error : Com Library Initialize Security Failed." );
        return false;
    }

    // ウィンドウの初期化.
    if ( !InitWnd() )
    {
        DLOG( "Error : InitWnd() Failed." );
        return false;
    }

    // Direct3Dの初期化.
    if ( !InitD3D() )
    {
        DLOG( "Error : InitD3D() Failed." );
        return false;
    }

    // Direct2Dの初期化.
    if ( !InitD2D() )
    {
        ELOG( "Error : InitD2D() Failed." );
        return false;
    }

    // アプリケーション固有の初期化.
    if ( !OnInit() )
    {
        ELOG( "Error : OnInit() Failed." );
        return false;
    }

    // ウィンドウを表示します.
    ShowWindow( m_hWnd, SW_SHOWNORMAL );
    UpdateWindow( m_hWnd );

    // フォーカスを設定します.
    SetFocus( m_hWnd );

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      アプリケーションの終了処理.
//-----------------------------------------------------------------------------
void Application::TermApp()
{
    // ステートをデフォルトに戻します.
    if ( m_pDeviceContext )
    {
        m_pDeviceContext->ClearState();
        m_pDeviceContext->Flush();
    }

    // アプリケーション固有の終了処理.
    OnTerm();

    // Direct2Dの終了処理.
    TermD2D();

    // Direct3Dの終了処理.
    TermD3D();

    // ウィンドウの終了処理.
    TermWnd();

    // COMライブラリの終了処理.
    CoUninitialize();

    // タイマー分解能を戻す.
    timeEndPeriod(1);
}

//-----------------------------------------------------------------------------
//      ウィンドウの初期化処理.
//-----------------------------------------------------------------------------
bool Application::InitWnd()
{
    // インスタンスハンドルを取得.
    HINSTANCE hInst = GetModuleHandle( nullptr );
    if ( !hInst )
    {
        DLOG( "Error : GetModuleHandle() Failed. ");
        return false;
    }

    // アイコンなしの場合はロード.
    if ( m_hIcon == nullptr )
    { m_hIcon = LoadIcon( hInst, IDI_APPLICATION ); }

    // 拡張ウィンドウクラスの設定.
    WNDCLASSEXW wc = {};
    wc.cbSize           = sizeof( WNDCLASSEXW );
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MsgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = m_hIcon;
    wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = ASDX_WND_CLASSNAME;
    wc.hIconSm          = m_hIcon;

    // ウィンドウクラスを登録します.
    if ( !RegisterClassExW( &wc ) )
    {
        // エラーログ出力.
        DLOG( "Error : RegisterClassEx() Failed." );

        // 異常終了.
        return false;
    }

    // インスタンスハンドルを設定.
    m_hInst = hInst;

    // 矩形の設定.
    RECT rc = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

#if 0 // リサイズしたくない場合.
    //DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
#else // リサイズ許可.
    DWORD style = WS_OVERLAPPEDWINDOW;
#endif
    // 指定されたクライアント領域を確保するために必要なウィンドウ座標を計算します.
    AdjustWindowRect( &rc, style, FALSE );

    // ウィンドウを生成します.
    m_hWnd = CreateWindowW(
        ASDX_WND_CLASSNAME,
        m_Title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        ( rc.right - rc.left ),
        ( rc.bottom - rc.top ),
        NULL,
        m_hMenu,
        hInst,
        this
    );

    // 生成チェック.
    if ( !m_hWnd )
    {
        // エラーログ出力.
        DLOG( "Error : CreateWindow() Failed." );

        // 異常終了.
        return false;
    }

    // サウンドマネージャにハンドルを設定.
    SndMgr::GetInstance().SetHandle( m_hWnd );

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      ウィンドウの終了処理.
//-----------------------------------------------------------------------------
void Application::TermWnd()
{
    // ウィンドウクラスの登録を解除.
    if ( m_hInst != nullptr )
    { UnregisterClass( ASDX_WND_CLASSNAME, m_hInst ); }

    if ( m_hAccel )
    { DestroyAcceleratorTable( m_hAccel ); }

    if ( m_hMenu )
    { DestroyMenu( m_hMenu ); }

    if ( m_hIcon )
    { DestroyIcon( m_hIcon ); }

    // タイトル名をクリア.
    m_Title = nullptr;

    // ハンドルをクリア.
    m_hInst  = nullptr;
    m_hWnd   = nullptr;
    m_hIcon  = nullptr;
    m_hMenu  = nullptr;
    m_hAccel = nullptr;
}

//-----------------------------------------------------------------------------
//      Direct3Dの初期化処理.
//-----------------------------------------------------------------------------
bool Application::InitD3D()
{
    HRESULT hr = S_OK;

    // ウィンドウサイズを取得します.
    RECT rc;
    GetClientRect( m_hWnd, &rc );
    UINT w = rc.right - rc.left;
    UINT h = rc.bottom - rc.top;

    // 取得したサイズを設定します.
    m_Width       = w;
    m_Height      = h;

    // アスペクト比を算出します.
    m_AspectRatio = (FLOAT)w / (FLOAT)h;

    if (!DeviceContext::Instance().Init())
    {
        ELOG("Error : DeviceContext::Init() Failed.");
        return false;
    }

    m_pDevice         = DeviceContext::Instance().GetDevice();
    m_pDeviceContext  = DeviceContext::Instance().GetContext();
    auto pDXGIFactory = DeviceContext::Instance().GetDXGIFactory();

    // マルチサンプルクオリティの最大値を取得.
    if (m_EnableMultiSample)
    {
        uint32_t maxQualityLevel = 0;
        m_pDevice->CheckMultisampleQualityLevels( m_SwapChainFormat, m_MultiSampleCount, &maxQualityLevel );
        m_MultiSampleQuality = maxQualityLevel - 1;
    }
    else
    {
        m_MultiSampleCount   = 1;
        m_MultiSampleQuality = 0;
    }

    auto swapEffect = IsFlipEffect(m_SwapChainFormat) ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;

    // スワップチェインの構成設定.
    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.Width                = w;
    sd.Height               = h;
    sd.Format               = m_SwapChainFormat;
    sd.Stereo               = FALSE;
    sd.SampleDesc.Count     = m_MultiSampleCount;
    sd.SampleDesc.Quality   = m_MultiSampleQuality;
    sd.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sd.BufferCount          = m_SwapChainCount;
    sd.Scaling              = DXGI_SCALING_STRETCH;
    sd.SwapEffect           = swapEffect;
    sd.AlphaMode            = DXGI_ALPHA_MODE_UNSPECIFIED;

    hr = pDXGIFactory->CreateSwapChainForHwnd(m_pDevice, m_hWnd, &sd, nullptr, nullptr, m_pSwapChain.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIFactory::CreateSwapChain() Failed. errcode = 0x%x", hr);
        return false;
    }

    // IDXGISwapChain4にキャスト.
    hr = m_pSwapChain->QueryInterface(IID_PPV_ARGS(m_pSwapChain4.GetAddress()));
    if ( FAILED( hr ) )
    {
        m_pSwapChain4.Reset();
        ELOG( "Warning : IDXGISwapChain4 Conversion Faild.");
    }
    else
    {
        // HDR出力チェック.
        CheckSupportHDR();
    }

#if ASDX_IS_DEBUG
    // デバッグオブジェクトを初期化.
    m_pDevice->QueryInterface(IID_PPV_ARGS(m_pD3D11Debug.GetAddress()));
#endif//ASDX_IS_DEBUG

    // レンダーターゲットを生成.
    if ( !m_ColorTarget2D.CreateFromBackBuffer( m_pDevice, m_pSwapChain.GetPtr() ) )
    {
        DLOG( "Error : RenderTarget2D::CreateFromBackBuffer() Failed." );
        return false;
    }

    // 深度ステンシルバッファの構成設定.
    TargetDesc2D desc;
    desc.Width              = w;
    desc.Height             = h;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = m_DepthStencilFormat;
    desc.SampleDesc.Count   = m_MultiSampleCount;
    desc.SampleDesc.Quality = m_MultiSampleQuality;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;

    // 深度ステンシルバッファを生成.
    if ( !m_DepthTarget2D.Create( m_pDevice, desc ) )
    {
        DLOG( "Error : DepthStencilTarget::Create() Failed." );
        return false;
    }

    // デバイスコンテキストにレンダーターゲットを設定.
    ID3D11RenderTargetView* pRTV = m_ColorTarget2D.GetTargetView();
    ID3D11DepthStencilView* pDSV = m_DepthTarget2D.GetTargetView();
    m_pDeviceContext->OMSetRenderTargets( 1, &pRTV, pDSV );

    // ビューポートの設定.
    m_Viewport.Width    = (FLOAT)w;
    m_Viewport.Height   = (FLOAT)h;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;

    // シザー矩形の設定.
    m_ScissorRect.left   = 0;
    m_ScissorRect.right  = w;
    m_ScissorRect.top    = 0;
    m_ScissorRect.bottom = h;

    // デバイスコンテキストにビューポートを設定.
    m_pDeviceContext->RSSetViewports( 1, &m_Viewport );

    // デバイスコンテキストにシザー矩形を設定.
    m_pDeviceContext->RSSetScissorRects( 1, &m_ScissorRect );

    return true;
}

//-----------------------------------------------------------------------------
//      Direct3Dの終了処理.
//-----------------------------------------------------------------------------
void Application::TermD3D()
{
    // 描画ターゲットを解放.
    m_ColorTarget2D.Release();

    // 深度ステンシルバッファを解放.
    m_DepthTarget2D.Release();

    // スワップチェインを解放.
    m_pSwapChain.Reset();

    m_pDevice        = nullptr;
    m_pDeviceContext = nullptr;

    DeviceContext::Instance().Term();

#if ASDX_IS_DEBUG
    //// メモリリークが出た場合は、下記をコメントアウトすれば詳細情報が表示される.
    //if( m_pD3D11Debug )
    //{ m_pD3D11Debug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL ); }
    m_pD3D11Debug.Reset();
#endif//ASDX_IS_DEBUG
}

//-----------------------------------------------------------------------------
//      Direct2D の初期化処理です.
//-----------------------------------------------------------------------------
bool Application::InitD2D()
{
#if defined(ASDX_ENABLE_D2D)
    HRESULT hr = S_OK;

    // D2Dファクトリーを生成.
    hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, m_pFactory2D.GetAddress() );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : D2D1CreateFactory() Failed." );
        return false;
    }

    // DirectWriteファクトリーを生成.
    hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( m_pFactoryDW.GetPtr() ), reinterpret_cast<IUnknown**>( m_pFactoryDW.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : DWriteCreateFactory() Failed." );
        return false;
    }

    static const WCHAR fontName[] = L"メイリオ";
    static const FLOAT fontSize = 14.0;

    // テキストフォーマットの生成.
    hr = m_pFactoryDW->CreateTextFormat(
        fontName,
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"",
        m_pDefaultTextFormat.GetAddress() );

    if ( FAILED( hr ) )
    {
        ELOG( "Error : IDWriteFactory::CreateTextFormat() Failed." );
        return false;
    }

    // D2Dデバイスの生成.
    hr = m_pFactory2D->CreateDevice( DeviceContext::Instance().GetDXGIDevice(), m_pDevice2D.GetAddress() );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID2D1Factory1::CreateDevice() Failed." );
        return false;
    }

    // D2Dデバイスコンテキストの生成.
    hr = m_pDevice2D->CreateDeviceContext( D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_pDeviceContext2D.GetAddress() );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID2D1Device::CreateDeviceContext() Failed." );
        return false;
    }

    // カラーブラシを生成.
    hr = m_pDeviceContext2D->CreateSolidColorBrush( D2D1::ColorF(D2D1::ColorF::White), m_pDefaultBrush.GetAddress() );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID2D1DeviceContext::CreateSolidBrush() Failed" );
        return false;
    }
#endif//defined(ASDX_ENABLE_D2D)

    return true;
}

//-----------------------------------------------------------------------------
//      Direct2D の終了処理です.
//-----------------------------------------------------------------------------
void Application::TermD2D()
{
#if defined(ASDX_ENABLE_D2D)
    m_pDefaultTextFormat.Reset();
    m_pFactoryDW        .Reset();

    m_pDefaultBrush   .Reset();
    m_pDeviceContext2D.Reset();
    m_pDevice2D       .Reset(); 
    m_pFactory2D      .Reset();
//    m_pBitmap2D       .Reset();
#endif//defined(ASDX_ENABLE_D2D)
}

//-----------------------------------------------------------------------------
//      メインループ処理.
//-----------------------------------------------------------------------------
void Application::MainLoop()
{
    MSG msg = { 0 };

    auto frameCount = 0;

    while( WM_QUIT != msg.message )
    {
        auto gotMsg = PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE );

        if ( gotMsg )
        {
            auto ret = TranslateAccelerator( m_hWnd, m_hAccel, &msg );
            if ( 0 == ret )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
        else
        {
            // デバイス・デバイスコンテキスト・スワップチェインのいずれかがNULLであればスキップ.
            if ( ( m_pDevice        == nullptr )
              || ( m_pDeviceContext == nullptr )
              || ( m_pSwapChain     == nullptr ) )
            { continue; }

            // 時間を取得.
            m_Timer.End();
            auto time = m_Timer.GetElapsedSec();

            // 0.5秒ごとにFPSを更新.
            auto interval = float( time - m_LatestUpdateTime );
            if ( interval > 0.5 )
            {
                // FPSを算出.
                m_FPS = frameCount / interval;

                // 更新時間を設定.
                m_LatestUpdateTime = time;

                frameCount = 0;
            }

            // フレーム遷移処理.
            OnFrameMove();

            // フレーム描画処理.
            OnFrameRender();

            // フレームカウントをインクリメント.
            m_FrameCount++;
            frameCount++;
        }
    }
}

//-----------------------------------------------------------------------------
//      アプリケーションを実行します.
//-----------------------------------------------------------------------------
void Application::Run()
{
    // アプリケーションの初期化処理.
    if ( InitApp() )
    {
        // メインループ処理.
        MainLoop();
    }

    // アプリケーションの終了処理.
    TermApp();
}

//-----------------------------------------------------------------------------
//      キーイベント処理.
//-----------------------------------------------------------------------------
void Application::KeyEvent( const KeyEventArgs& param )
{
    // キーイベント呼び出し.
    OnKey( param );
}

//-----------------------------------------------------------------------------
//      リサイズイベント処理.
//-----------------------------------------------------------------------------
void Application::ResizeEvent( const ResizeEventArgs& param )
{
    // ウインドウ非表示状態に移行する時に縦横1ピクセルのリサイズイベントが発行される
    // マルチサンプル等の関係で縦横1ピクセルは問題が起こるので処理をスキップ
    if ( param.Width == 1 && param.Height == 1 )
    { return; }

    m_Width       = param.Width;
    m_Height      = param.Height;
    m_AspectRatio = param.AspectRatio;

    if ( ( m_pSwapChain     != nullptr )
      && ( m_pDeviceContext != nullptr ) )
    {
        // リサイズ前に一度コマンドを実行(実行しないとメモリリークするぽい).
        m_pSwapChain->Present( 0, 0 );

        ID3D11RenderTargetView* pNull = nullptr;
        m_pDeviceContext->OMSetRenderTargets( 1, &pNull, nullptr );

        // 描画ターゲットを解放.
        m_ColorTarget2D.Release();

        // 深度ステンシルバッファを解放.
        m_DepthTarget2D.Release();

        HRESULT hr = S_OK;

        // バッファをリサイズ.
        hr = m_pSwapChain->ResizeBuffers( m_SwapChainCount, 0, 0, m_SwapChainFormat, 0 );
        if ( FAILED( hr ) )
        { DLOG( "Error : IDXGISwapChain::ResizeBuffer() Failed." ); }

        // バックバッファから描画ターゲットを生成.
        if ( !m_ColorTarget2D.CreateFromBackBuffer( m_pDevice, m_pSwapChain.GetPtr() ) )
        { DLOG( "Error : RenderTarget2D::CreateFromBackBuffer() Failed." ); }

        TargetDesc2D desc;
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = m_DepthStencilFormat;
        desc.SampleDesc.Count   = m_MultiSampleCount;
        desc.SampleDesc.Quality = m_MultiSampleQuality;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        if ( !m_DepthTarget2D.Create( m_pDevice, desc ) )
        { DLOG( "Error : DepthStencilTarget::Create() Failed." ); }

        // デバイスコンテキストにレンダーターゲットを設定.
        ID3D11RenderTargetView* pRTV = m_ColorTarget2D.GetTargetView();
        ID3D11DepthStencilView* pDSV = m_DepthTarget2D.GetTargetView();
        m_pDeviceContext->OMSetRenderTargets( 1, &pRTV, pDSV );

        // ビューポートの設定.
        m_Viewport.Width    = (FLOAT)m_Width;
        m_Viewport.Height   = (FLOAT)m_Height;
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;
        m_Viewport.TopLeftX = 0;
        m_Viewport.TopLeftY = 0;

        // シザー矩形の設定.
        m_ScissorRect.left   = 0;
        m_ScissorRect.right  = m_Width;
        m_ScissorRect.top    = 0;
        m_ScissorRect.bottom = m_Height;

        // デバイスコンテキストにビューポートを設定.
        m_pDeviceContext->RSSetViewports( 1, &m_Viewport );

        // デバイスコンテキストにシザー矩形を設定.
        m_pDeviceContext->RSSetScissorRects( 1, &m_ScissorRect );
    }

    // リサイズイベント呼び出し.
    OnResize( param );
}

//-----------------------------------------------------------------------------
//      マウスイベント処理.
//-----------------------------------------------------------------------------
void Application::MouseEvent( const MouseEventArgs& param )
{
    OnMouse( param );
}

//-----------------------------------------------------------------------------
//      ドロップイベント処理.
//------------------------------------------------------------------------------
void Application::DropEvent( const std::vector<std::string>& dropFiles )
{
    OnDrop( dropFiles );
}

//-----------------------------------------------------------------------------
//      ウィンドウプロシージャ.
//-----------------------------------------------------------------------------
LRESULT CALLBACK Application::MsgProc( HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp )
{
    auto pInstance = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    PAINTSTRUCT ps;
    HDC         hdc;

    if ( ( uMsg == WM_KEYDOWN )
      || ( uMsg == WM_SYSKEYDOWN )
      || ( uMsg == WM_KEYUP )
      || ( uMsg == WM_SYSKEYUP ) )
    {
        bool isKeyDown = ( uMsg == WM_KEYDOWN  || uMsg == WM_SYSKEYDOWN );

        DWORD mask = ( 1 << 29 );
        bool isAltDown =( ( lp & mask ) != 0 );

        KeyEventArgs args;
        args.KeyCode   = uint32_t( wp );
        args.IsAltDown = isAltDown;
        args.IsKeyDown = isKeyDown;

        if (pInstance != nullptr)
        { pInstance->KeyEvent(args); }
    }

    // 古いWM_MOUSEWHEELの定義.
    const UINT OLD_WM_MOUSEWHEEL = 0x020A;

    if ( ( uMsg == WM_LBUTTONDOWN )
      || ( uMsg == WM_LBUTTONUP )
      || ( uMsg == WM_LBUTTONDBLCLK )
      || ( uMsg == WM_MBUTTONDOWN )
      || ( uMsg == WM_MBUTTONUP )
      || ( uMsg == WM_MBUTTONDBLCLK )
      || ( uMsg == WM_RBUTTONDOWN )
      || ( uMsg == WM_RBUTTONUP )
      || ( uMsg == WM_RBUTTONDBLCLK )
      || ( uMsg == WM_XBUTTONDOWN )
      || ( uMsg == WM_XBUTTONUP )
      || ( uMsg == WM_XBUTTONDBLCLK )
      || ( uMsg == WM_MOUSEHWHEEL )             // このWM_MOUSEWHEELは0x020Eを想定.
      || ( uMsg == WM_MOUSEMOVE )
      || ( uMsg == OLD_WM_MOUSEWHEEL ) )
    {
        int x = (short)LOWORD( lp );
        int y = (short)HIWORD( lp );

        int wheelDelta = 0;
        if ( ( uMsg == WM_MOUSEHWHEEL )
          || ( uMsg == OLD_WM_MOUSEWHEEL ) )
        {
            POINT pt = {};
            pt.x = x;
            pt.y = y;

            ScreenToClient( hWnd, &pt );
            x = pt.x;
            y = pt.y;

            wheelDelta += (short)HIWORD( wp );
        }

        int  buttonState = LOWORD( wp );
        bool isLeftButtonDown   = ( ( buttonState & MK_LBUTTON  ) != 0 );
        bool isRightButtonDown  = ( ( buttonState & MK_RBUTTON  ) != 0 );
        bool isMiddleButtonDown = ( ( buttonState & MK_MBUTTON  ) != 0 );
        bool isSideButton1Down  = ( ( buttonState & MK_XBUTTON1 ) != 0 );
        bool isSideButton2Down  = ( ( buttonState & MK_XBUTTON2 ) != 0 );

        MouseEventArgs args;
        args.X = x;
        args.Y = y;
        args.WheelDelta         = wheelDelta;
        args.IsLeftButtonDown   = isLeftButtonDown;
        args.IsMiddleButtonDown = isMiddleButtonDown;
        args.IsRightButtonDown  = isRightButtonDown;
        args.IsSideButton1Down  = isSideButton1Down;
        args.IsSideButton2Down  = isSideButton2Down;

        if (pInstance != nullptr)
        { pInstance->MouseEvent(args); }
    }

    switch( uMsg )
    {
    case WM_CREATE:
        {
            auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lp);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));

            // ドラッグアンドドロップ可能.
            DragAcceptFiles(hWnd, TRUE);
        }
        break;

    case WM_PAINT:
        {
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
        }
        break;

    case WM_DESTROY:
        { PostQuitMessage( 0 ); }
        break;

    case WM_SIZE:
        {
            UINT w = (UINT)LOWORD( lp );
            UINT h = (UINT)HIWORD( lp );

            // ウインドウ非表示状態に移行する時に縦横1ピクセルのリサイズイベントが発行される
            // マルチサンプル等の関係で縦横1ピクセルは問題が起こるので最少サイズを設定
            ResizeEventArgs args;
            args.Width  = asdx::Max( w, (uint32_t)8 );
            args.Height = asdx::Max( h, (uint32_t)8 );
            args.AspectRatio = float( args.Width ) / args.Height;

            if (pInstance != nullptr)
            { pInstance->ResizeEvent(args); }
        }
        break;

    case WM_DROPFILES:
        {
            // ドロップされたファイル数を取得.
            uint32_t fileCount = DragQueryFileA((HDROP)wp, 0xFFFFFFFF, NULL, 0);

            // 作業用のバッファを確保.
            std::vector<std::string> dropFiles;
            dropFiles.resize(fileCount);

            for (uint32_t i=0; i <fileCount; i++)
            {
                char dropFile[MAX_PATH] = {};

                // ドロップされたファイル名を取得.
                DragQueryFileA((HDROP)wp, i, dropFile, MAX_PATH);
                dropFiles[ i ] = dropFile;
            }

            // アプリケーションに通知.
            if (pInstance != nullptr)
            { pInstance->DropEvent(dropFiles); }

            dropFiles.clear();

            DragFinish((HDROP)wp);
        }
        break;

    case WM_MOVE:
        {
            if (pInstance != nullptr)
            { pInstance->CheckSupportHDR(); }
        }
        break;

    case WM_DISPLAYCHANGE:
        {
            if (pInstance != nullptr)
            { pInstance->CheckSupportHDR(); }
        }
        break;

    case WM_CHAR:
        {
            if (pInstance != nullptr)
            {
                auto keyCode = static_cast<uint32_t>(wp);
                pInstance->OnTyping(keyCode);
            }
        }
        break;

    case MM_MCINOTIFY:
        {
            // サウンドマネージャのコールバック.
            SndMgr::GetInstance().OnNofity( uint32_t(lp), uint32_t(wp) );
        }
        break;
    }

    // ユーザーカスタマイズ用に呼び出し.
    if (pInstance != nullptr)
    { pInstance->OnMsgProc(hWnd, uMsg, wp, lp); }

    return DefWindowProc( hWnd, uMsg, wp, lp );
}

//-----------------------------------------------------------------------------
//      初期化時の処理.
//-----------------------------------------------------------------------------
bool Application::OnInit()
{
    /* DO_NOTHING */
    return true;
}

//-----------------------------------------------------------------------------
//      終了時の処理.
//-----------------------------------------------------------------------------
void Application::OnTerm()
{
    /* DO_NOTHING */
}

//-----------------------------------------------------------------------------
//      フレーム遷移時の処理.
//-----------------------------------------------------------------------------
void Application::OnFrameMove()
{
    /* DO_NOTHING */
}

//-----------------------------------------------------------------------------
//      フレーム描画字の処理.
//-----------------------------------------------------------------------------
void Application::OnFrameRender()
{
    // レンダーターゲットビュー・深度ステンシルビューを取得.
    ID3D11RenderTargetView* pRTV = m_ColorTarget2D.GetTargetView();
    ID3D11DepthStencilView* pDSV = m_DepthTarget2D.GetTargetView();

    // NULLチェック.
    if ( pRTV == nullptr )
    { return; }
    if ( pDSV == nullptr )
    { return; }

    // 出力マネージャに設定.
    m_pDeviceContext->OMSetRenderTargets( 1, &pRTV, pDSV );

    // クリア処理.
    m_pDeviceContext->ClearRenderTargetView( pRTV, m_ClearColor );
    m_pDeviceContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    // 描画処理.
    {
        /* DO_NOTHING */
    }

    // コマンドを実行して，画面に表示.
    Present( 0 );
}

//-----------------------------------------------------------------------------
//      コマンドを実行して，画面に表示します.
//-----------------------------------------------------------------------------
void Application::Present( uint32_t syncInterval )
{
    HRESULT hr = S_OK;

    auto pSwapChain = (m_pSwapChain4.GetPtr()) 
        ? m_pSwapChain4.GetPtr()
        : m_pSwapChain.GetPtr();

    // スタンバイモードかどうかチェック.
    if ( m_IsStandbyMode )
    {
        // テストする.
        hr = pSwapChain->Present( syncInterval, DXGI_PRESENT_TEST );

        // スタンバイモードが解除されたかをチェック.
        if ( hr == S_OK )
        { m_IsStandbyMode = false; }

        // 処理を中断.
        return;
    }

    // 画面更新する.
    hr = pSwapChain->Present( syncInterval, 0 );

    switch( hr )
    {
    // デバイスがリセットされた場合(=コマンドが正しくない場合)
    case DXGI_ERROR_DEVICE_RESET:
        {
            // エラーログ出力.
            ELOG( "Fatal Error : IDXGISwapChain::Present() Failed. ErrorCode = DXGI_ERROR_DEVICE_RESET." );

            // 続行できないのでダイアログを表示.
            MessageBoxW( m_hWnd, L"A Fatal Error Occured. Shutting down.", L"FATAL ERROR", MB_OK | MB_ICONERROR );

            // 終了メッセージを送る.
            PostQuitMessage( 1 );
        }
        break;

    // デバイスが削除された場合(=GPUがぶっこ抜かれた場合かドライバーアップデート中)
    case DXGI_ERROR_DEVICE_REMOVED:
        {
            // エラーログ出力.
            ELOG( "Fatal Error : IDXGISwapChain::Present() Failed. ErrorCode = DXGI_ERROR_DEVICE_REMOVED." );

            // 続行できないのでダイアログを表示.
            MessageBoxW( m_hWnd, L"A Fatal Error Occured. Shutting down.", L"FATAL ERROR", MB_OK | MB_ICONERROR );

            // 終了メッセージを送る.
            PostQuitMessage( 2 );
        }
        break;

    // 表示領域がなければスタンバイモードに入る.
    case DXGI_STATUS_OCCLUDED:
        { m_IsStandbyMode = true; }
        break;

    // 現在のフレームバッファを表示する場合.
    case S_OK:
        { /* DO_NOTHING */ }
        break;
    }
}

//-----------------------------------------------------------------------------
//      ディスプレイがHDR出力をサポートしているかどうかチェックします.
//-----------------------------------------------------------------------------
void Application::CheckSupportHDR()
{
    // 何も作られていない場合は処理しない.
    if (m_pSwapChain4 == nullptr || m_pDevice == nullptr)
    { return; }

    HRESULT hr = S_OK;

    // ウィンドウ領域を取得.
    RECT rect;
    GetWindowRect(m_hWnd, &rect);

    RefPtr<IDXGIFactory5> pFactory;
    hr = CreateDXGIFactory2(0, IID_PPV_ARGS(pFactory.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : CreateDXGIFactory2() Failed.");
        return;
    }

    RefPtr<IDXGIAdapter1> pAdapter;
    hr = pFactory->EnumAdapters1(0, pAdapter.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIFactory5::EnumAdapters1() Failed.");
        return;
    }

    UINT i = 0;
    RefPtr<IDXGIOutput> currentOutput;
    RefPtr<IDXGIOutput> bestOutput;
    int bestIntersectArea = -1;

    // 各ディスプレイを調べる.
    while (pAdapter->EnumOutputs(i, currentOutput.GetAddress()) != DXGI_ERROR_NOT_FOUND)
    {
        auto ax1 = rect.left;
        auto ay1 = rect.top;
        auto ax2 = rect.right;
        auto ay2 = rect.bottom;

        // ディスプレイの設定を取得.
        DXGI_OUTPUT_DESC desc;
        hr = currentOutput->GetDesc(&desc);
        if (FAILED(hr))
        { return; }

        auto bx1 = desc.DesktopCoordinates.left;
        auto by1 = desc.DesktopCoordinates.top;
        auto bx2 = desc.DesktopCoordinates.right;
        auto by2 = desc.DesktopCoordinates.bottom;

        // 領域が一致するかどうか調べる.
        int intersectArea = ComputeIntersectionArea(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2);
        if (intersectArea > bestIntersectArea)
        {
            bestOutput = currentOutput;
            bestIntersectArea = intersectArea;
        }

        i++;
    }

    // 一番適しているディスプレイ.
    RefPtr<IDXGIOutput6> pOutput6;
    hr = bestOutput->QueryInterface(IID_PPV_ARGS(pOutput6.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIOutput6 Conversion Failed.");
        return;
    }

    // 出力設定を取得.
    hr = pOutput6->GetDesc1(&m_DisplayDesc);
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIOutput6::GetDesc() Failed.");
        return;
    }

    // 正常終了.
}

//-----------------------------------------------------------------------------
//      HDR出力をサポートしているかどうかチェックします.
//-----------------------------------------------------------------------------
bool Application::IsSupportHDR() const
{ return m_DisplayDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020; }

//-----------------------------------------------------------------------------
//      ディスプレイ設定を取得します.
//-----------------------------------------------------------------------------
DXGI_OUTPUT_DESC1 Application::GetDisplayDesc() const
{ return m_DisplayDesc; }

//-----------------------------------------------------------------------------
//      色空間を設定します
//-----------------------------------------------------------------------------
bool Application::SetColorSpace(COLOR_SPACE value)
{
    if (m_pSwapChain4.GetPtr() == nullptr)
    { return false; }

    DXGI_HDR_METADATA_HDR10 metaData = {};
    DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

    metaData.MinMasteringLuminance = GetLuma(m_DisplayDesc.MinLuminance);
    metaData.MaxMasteringLuminance = GetLuma(m_DisplayDesc.MaxLuminance);

    switch (value)
    {
    case COLOR_SPACE_NONE:
        {
            colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
            metaData.RedPrimary[0]   = GetCoord(m_DisplayDesc.RedPrimary[0]);
            metaData.RedPrimary[1]   = GetCoord(m_DisplayDesc.RedPrimary[1]);
            metaData.GreenPrimary[0] = GetCoord(m_DisplayDesc.GreenPrimary[0]);
            metaData.GreenPrimary[1] = GetCoord(m_DisplayDesc.GreenPrimary[1]);
            metaData.BluePrimary[0]  = GetCoord(m_DisplayDesc.BluePrimary[0]);
            metaData.BluePrimary[1]  = GetCoord(m_DisplayDesc.BluePrimary[1]);
            metaData.WhitePoint[0]   = GetCoord(m_DisplayDesc.WhitePoint[0]);
            metaData.WhitePoint[1]   = GetCoord(m_DisplayDesc.WhitePoint[1]);
        }
        break;

    case COLOR_SPACE_SRGB:
        {
            colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
            metaData.RedPrimary[0]   = GetCoord(0.64000f);
            metaData.RedPrimary[1]   = GetCoord(0.33000f);
            metaData.GreenPrimary[0] = GetCoord(0.30000f);
            metaData.GreenPrimary[1] = GetCoord(0.60000f);
            metaData.BluePrimary[0]  = GetCoord(0.15000f);
            metaData.BluePrimary[1]  = GetCoord(0.06000f);
            metaData.WhitePoint[0]   = GetCoord(0.31270f);
            metaData.WhitePoint[1]   = GetCoord(0.32900f);
        }
        break;

    case COLOR_SPACE_BT709:
        {
            colorSpace = DXGI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P709;
            metaData.RedPrimary[0]   = GetCoord(0.64000f);
            metaData.RedPrimary[1]   = GetCoord(0.33000f);
            metaData.GreenPrimary[0] = GetCoord(0.30000f);
            metaData.GreenPrimary[1] = GetCoord(0.60000f);
            metaData.BluePrimary[0]  = GetCoord(0.15000f);
            metaData.BluePrimary[1]  = GetCoord(0.06000f);
            metaData.WhitePoint[0]   = GetCoord(0.31270f);
            metaData.WhitePoint[1]   = GetCoord(0.32900f);
        }
        break;

    case COLOR_SPACE_BT2100_PQ:
        {
            colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            metaData.RedPrimary[0]   = GetCoord(0.70800f);
            metaData.RedPrimary[1]   = GetCoord(0.29200f);
            metaData.GreenPrimary[0] = GetCoord(0.17000f);
            metaData.GreenPrimary[1] = GetCoord(0.79700f);
            metaData.BluePrimary[0]  = GetCoord(0.13100f);
            metaData.BluePrimary[1]  = GetCoord(0.04600f);
            metaData.WhitePoint[0]   = GetCoord(0.31270f);
            metaData.WhitePoint[1]   = GetCoord(0.32900f);
        }
        break;

    case COLOR_SPACE_BT2100_HLG:
        {
            colorSpace = DXGI_COLOR_SPACE_YCBCR_FULL_GHLG_TOPLEFT_P2020;
            metaData.RedPrimary[0]   = GetCoord(0.70800f);
            metaData.RedPrimary[1]   = GetCoord(0.29200f);
            metaData.GreenPrimary[0] = GetCoord(0.17000f);
            metaData.GreenPrimary[1] = GetCoord(0.79700f);
            metaData.BluePrimary[0]  = GetCoord(0.13100f);
            metaData.BluePrimary[1]  = GetCoord(0.04600f);
            metaData.WhitePoint[0]   = GetCoord(0.31270f);
            metaData.WhitePoint[1]   = GetCoord(0.32900f);
        }
        break;
    }

    UINT flag = 0;
    auto hr = m_pSwapChain4->CheckColorSpaceSupport(colorSpace, &flag);
    if (FAILED(hr))
    { return false; }

    if ((flag & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) != DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)
    { return false; }

    hr = m_pSwapChain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(metaData), &metaData);
    if (FAILED(hr))
    { return false; }

    hr = m_pSwapChain4->SetColorSpace1(colorSpace);
    if (FAILED(hr))
    { return false; }

    return true;
}

//-----------------------------------------------------------------------------
//      ディスプレイのリフレッシュレートを取得します.
//-----------------------------------------------------------------------------
bool Application::GetDisplayRefreshRate(DXGI_RATIONAL& result) const
{
    asdx::RefPtr<IDXGIOutput> output;
    auto hr = m_pSwapChain->GetContainingOutput(output.GetAddress());
    if (FAILED(hr))
    { return false; }

    DXGI_OUTPUT_DESC outputDesc;
    hr = output->GetDesc(&outputDesc);
    if (FAILED(hr))
    { return false; }

    auto hMonitor = outputDesc.Monitor;

    MONITORINFOEX monitorInfo;
    monitorInfo.cbSize = sizeof(monitorInfo);
    auto ret = GetMonitorInfo(hMonitor, &monitorInfo);
    if (ret == 0)
    { return false; }
    
    DEVMODE devMode;
    devMode.dmSize          = sizeof(devMode);
    devMode.dmDriverExtra   = 0;
    ret = EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
    if (ret == 0)
    { return false; }
    
    auto useDefaultRefreshRate = (1 == devMode.dmDisplayFrequency) || (0 == devMode.dmDisplayFrequency);
    result.Numerator   = (useDefaultRefreshRate) ? 0 : devMode.dmDisplayFrequency;
    result.Denominator = (useDefaultRefreshRate) ? 0 : 1;

    return true;
}

//-----------------------------------------------------------------------------
//      リサイズ時の処理.
//-----------------------------------------------------------------------------
void Application::OnResize( const ResizeEventArgs& )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      キーイベント時の処理.
//-----------------------------------------------------------------------------
void Application::OnKey( const KeyEventArgs& )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      マウスイベント時の処理.
//-----------------------------------------------------------------------------
void Application::OnMouse( const MouseEventArgs& )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      タイピングイベント時の処理.
//-----------------------------------------------------------------------------
void Application::OnTyping( uint32_t )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      ドロップ時の処理.
//------------------------------------------------------------------------------
void Application::OnDrop( const std::vector<std::string>& )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      メッセージプロシージャの処理.
//-----------------------------------------------------------------------------
void Application::OnMsgProc( HWND, UINT, WPARAM, LPARAM )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      フォーカスを持つかどうか判定します.
//-----------------------------------------------------------------------------
bool Application::HasFocus() const
{ return ( GetActiveWindow() == m_hWnd ); }

//------------------------------------------------------------------------------
//      スタンバイモードかどうかチェックします.
//------------------------------------------------------------------------------
bool Application::IsStandByMode() const
{ return m_IsStandbyMode; }

//-----------------------------------------------------------------------------
//      デバイスを取得します.
//-----------------------------------------------------------------------------
ID3D11Device* Application::GetDevice() const
{ return m_pDevice; }

//-----------------------------------------------------------------------------
//      デバイスコンテキストを取得します.
//-----------------------------------------------------------------------------
ID3D11DeviceContext4* Application::GetDeviceContext() const
{ return m_pDeviceContext; }

} // namespace asdx
