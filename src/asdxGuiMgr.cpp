//-----------------------------------------------------------------------------
// File : asdxGuiMgr.cpp
// Desc : Gui Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxGuiMgr.h>

#ifdef ASDX_ENABLE_IMGUI
#include <d3dcompiler.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <asdxMisc.h>
#include <codecvt>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/ImguiVS.inc"
#include "../res/shaders/Compiled/ImguiPS.inc"
#include "../res/shaders/Compiled/ImguiCubePS.inc"

static const ImWchar glyphRangesJapanese[] = {
    #include "GlyphJapanese.h"
};


///////////////////////////////////////////////////////////////////////////////
// TransformBuffer
///////////////////////////////////////////////////////////////////////////////
struct TransformBuffer
{
    float WorldViewProjection[ 4 ][ 4 ];
};

//-----------------------------------------------------------------------------
//      クリップボードテキストを取得します.
//-----------------------------------------------------------------------------
const char* GetClipboardText(void*)
{
    static char* buf_local = NULL;
    if (buf_local)
    {
        ImGui::MemFree(buf_local);
        buf_local = NULL;
    }
    if (!OpenClipboard(NULL))
        return NULL;
    HANDLE wbuf_handle = GetClipboardData(CF_UNICODETEXT);
    if (wbuf_handle == NULL)
        return NULL;
    if (ImWchar* wbuf_global = (ImWchar*)GlobalLock(wbuf_handle))
    {
        int buf_len = ImTextCountUtf8BytesFromStr(wbuf_global, NULL) + 1;
        buf_local = (char*)ImGui::MemAlloc(buf_len * sizeof(char));
        ImTextStrToUtf8(buf_local, buf_len, wbuf_global, NULL);
    }
    GlobalUnlock(wbuf_handle);
    CloseClipboard();
    return buf_local;
}

//-----------------------------------------------------------------------------
//      クリップボードテキストを設定します.
//-----------------------------------------------------------------------------
void SetClipboardText(void*, const char* text)
{
    if (!OpenClipboard(NULL))
        return;
    const int wbuf_length = ImTextCountCharsFromUtf8(text, NULL) + 1;
    HGLOBAL wbuf_handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(ImWchar));
    if (wbuf_handle == NULL)
        return;
    ImWchar* wbuf_global = (ImWchar*)GlobalLock(wbuf_handle);
    ImTextStrFromUtf8(wbuf_global, wbuf_length, text, NULL);
    GlobalUnlock(wbuf_handle);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, wbuf_handle);
    CloseClipboard();
}


} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GuiMgr class
///////////////////////////////////////////////////////////////////////////////
GuiMgr GuiMgr::s_Instance;

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GuiMgr::GuiMgr()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GuiMgr::~GuiMgr()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
GuiMgr& GuiMgr::GetInstance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期処理を行います.
//-----------------------------------------------------------------------------
bool GuiMgr::Init
(
    ID3D11Device*           pDevice,
    ID3D11DeviceContext*    pContext,
    HWND                    hWnd,
    uint32_t                width,
    uint32_t                height,
    const char*             fontPath
)
{
    m_pDevice  = pDevice;
    m_pContext = pContext;
    m_LastTime = std::chrono::system_clock::now();

    ImGui::CreateContext();

    auto& io = ImGui::GetIO();

    {
        std::string path;
        if (asdx::SearchFilePathA(fontPath, path))
        {
            auto utf8_path = asdx::ToStringUTF8(path);
            io.Fonts->AddFontFromFileTTF(utf8_path.c_str(), 12.0f, nullptr, glyphRangesJapanese);
        }

        uint8_t* pPixels;
        int width;
        int height;
        io.Fonts->GetTexDataAsRGBA32( &pPixels, &width, &height );

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count   = 1;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA res;
        res.pSysMem = pPixels;
        res.SysMemPitch = desc.Width * 4;
        res.SysMemSlicePitch = 0;

        auto hr = m_pDevice->CreateTexture2D( &desc, &res, m_pTexture.GetAddress() );
        if ( FAILED( hr ) )
        { return false; }

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
        viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipLevels = desc.MipLevels;
        viewDesc.Texture2D.MostDetailedMip = 0;

        hr = m_pDevice->CreateShaderResourceView( m_pTexture.GetPtr(), &viewDesc, m_pSRV.GetAddress() );
        if ( FAILED( hr ) )
        { return false; }

        io.Fonts->TexID = reinterpret_cast<void*>( m_pSRV.GetPtr() );
    }

    {
        D3D11_SAMPLER_DESC desc = {};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.MipLODBias = 0.0f;
        desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        desc.MinLOD = 0.0f;
        desc.MaxLOD = FLT_MAX;

        auto hr = m_pDevice->CreateSamplerState( &desc, m_pSmp.GetAddress() );
        if (FAILED(hr))
        { return false; }
    }

    // 頂点シェーダ
    {
        auto hr = m_pDevice->CreateVertexShader( ImguiVS, sizeof(ImguiVS), nullptr, m_pVS.GetAddress() );
        if (FAILED(hr))
        { return false; }

        D3D11_INPUT_ELEMENT_DESC elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)( &( (ImDrawVert*)0 )->pos ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)( &( (ImDrawVert*)0 )->uv ),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)( &( (ImDrawVert*)0 )->col ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = m_pDevice->CreateInputLayout( elements, 3, ImguiVS, sizeof(ImguiVS), m_pIL.GetAddress() );
        if (FAILED(hr))
        { return false; }
    }

    // ピクセルシェーダ.
    {
        auto hr = m_pDevice->CreatePixelShader( ImguiPS, sizeof(ImguiPS), nullptr, m_pPS.GetAddress() );
        if (FAILED(hr))
        { return false; }
    }

    // キューブマップ用ピクセルシェーダ.
    {
        auto hr = m_pDevice->CreatePixelShader(ImguiCubePS, sizeof(ImguiCubePS), nullptr, m_pPSCube.GetAddress());
        if (FAILED(hr))
        { return false; }
    }

    // 定数バッファ.
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof( TransformBuffer );
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        auto hr = m_pDevice->CreateBuffer( &desc, nullptr, m_pCB.GetAddress() );
        if ( FAILED( hr ) )
        { return false; }
    }

    // ブレンドステート.
    {
        D3D11_BLEND_DESC desc = {};
        desc.AlphaToCoverageEnable                      = false;
        desc.RenderTarget[ 0 ].BlendEnable              = true;
        desc.RenderTarget[ 0 ].SrcBlend                 = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[ 0 ].DestBlend                = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[ 0 ].BlendOp                  = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[ 0 ].SrcBlendAlpha            = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[ 0 ].DestBlendAlpha           = D3D11_BLEND_ZERO;
        desc.RenderTarget[ 0 ].BlendOpAlpha             = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[ 0 ].RenderTargetWriteMask    = D3D11_COLOR_WRITE_ENABLE_ALL;

        auto hr = m_pDevice->CreateBlendState( &desc, m_pBS.GetAddress() );
        if ( FAILED( hr ) )
        { return false; }
    }

    // ラスタライザーステート.
    {
        D3D11_RASTERIZER_DESC desc = {};
        desc.FillMode           = D3D11_FILL_SOLID;
        desc.CullMode           = D3D11_CULL_NONE;
        desc.ScissorEnable      = true;
        desc.DepthClipEnable    = true;

        auto hr = m_pDevice->CreateRasterizerState( &desc, m_pRS.GetAddress() );
        if ( FAILED( hr ) )
        { return false; }
    }

    // 深度ステンシルステート.
    {
        D3D11_DEPTH_STENCIL_DESC desc = {};
        desc.DepthEnable    = false;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc      = D3D11_COMPARISON_ALWAYS;
        desc.StencilEnable  = false;
        desc.FrontFace.StencilFailOp
            = desc.FrontFace.StencilDepthFailOp
            = desc.FrontFace.StencilPassOp
            = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc  = D3D11_COMPARISON_ALWAYS;
        desc.BackFace               = desc.FrontFace;

        auto hr = m_pDevice->CreateDepthStencilState( &desc, m_pDSS.GetAddress() );
        if ( FAILED( hr ) )
        { return false; }
    }

    // 頂点バッファ.
    {
        m_pVB.Reset();

        m_SizeVB = MaxPrimitiveCount * 4;

        D3D11_BUFFER_DESC desc = {};
        desc.Usage              = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth          = m_SizeVB * sizeof( ImDrawVert );
        desc.BindFlags          = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags     = D3D11_CPU_ACCESS_WRITE;

        auto hr = m_pDevice->CreateBuffer( &desc, nullptr, m_pVB.GetAddress() );
        if ( FAILED( hr ) )
        { return false; }
    }

    // インデックスバッファ.
    {
        m_pIB.Reset();

        m_SizeIB = MaxPrimitiveCount * 6;

        D3D11_BUFFER_DESC desc = {};
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth      = m_SizeIB * sizeof( ImDrawIdx );
        desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        auto hr = m_pDevice->CreateBuffer( &desc, nullptr, m_pIB.GetAddress() );
        if ( FAILED( hr ) )
        { return false; }
    }

    {
        io.KeyMap[ ImGuiKey_Tab ]       = VK_TAB;
        io.KeyMap[ ImGuiKey_LeftArrow ] = VK_LEFT;
        io.KeyMap[ ImGuiKey_RightArrow ]= VK_RIGHT;
        io.KeyMap[ ImGuiKey_UpArrow ]   = VK_UP;
        io.KeyMap[ ImGuiKey_DownArrow ] = VK_DOWN;
        io.KeyMap[ ImGuiKey_PageUp ]    = VK_PRIOR;
        io.KeyMap[ ImGuiKey_PageDown ]  = VK_NEXT;
        io.KeyMap[ ImGuiKey_Home ]      = VK_HOME;
        io.KeyMap[ ImGuiKey_End ]       = VK_END;
        io.KeyMap[ ImGuiKey_Delete ]    = VK_DELETE;
        io.KeyMap[ ImGuiKey_Backspace ] = VK_BACK;
        io.KeyMap[ ImGuiKey_Enter ]     = VK_RETURN;
        io.KeyMap[ ImGuiKey_Escape ]    = VK_ESCAPE;
        io.KeyMap[ ImGuiKey_A ]         = 'A';
        io.KeyMap[ ImGuiKey_C ]         = 'C';
        io.KeyMap[ ImGuiKey_V ]         = 'V';
        io.KeyMap[ ImGuiKey_X ]         = 'X';
        io.KeyMap[ ImGuiKey_Y ]         = 'Y';
        io.KeyMap[ ImGuiKey_Z ]         = 'Z';

        io.SetClipboardTextFn   = SetClipboardText;
        io.GetClipboardTextFn   = GetClipboardText;
        io.ImeWindowHandle      = hWnd;
        io.DisplaySize.x        = float( width );
        io.DisplaySize.y        = float( height );

        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.DeltaTime   = 1.0f / 60.0f;

        auto& style = ImGui::GetStyle();
        style.WindowRounding = 2.0f;

        style.Colors[ ImGuiCol_Text ]                   = ImVec4( 1.000000f, 1.000000f, 1.000000f, 1.000000f );
        style.Colors[ ImGuiCol_TextDisabled ]           = ImVec4( 0.400000f, 0.400000f, 0.400000f, 1.000000f );
        style.Colors[ ImGuiCol_WindowBg ]               = ImVec4( 0.060000f, 0.060000f, 0.060000f, 0.752000f );
        style.Colors[ ImGuiCol_PopupBg ]                = ImVec4( 0.000000f, 0.000000f, 0.000000f, 0.752000f );
        style.Colors[ ImGuiCol_Border ]                 = ImVec4( 1.000000f, 1.000000f, 1.000000f, 0.312000f );
        style.Colors[ ImGuiCol_BorderShadow ]           = ImVec4( 0.000000f, 0.000000f, 0.000000f, 0.080000f );
        style.Colors[ ImGuiCol_FrameBg ]                = ImVec4( 0.800000f, 0.800000f, 0.800000f, 0.300000f );
        style.Colors[ ImGuiCol_FrameBgHovered ]         = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.320000f );
        style.Colors[ ImGuiCol_FrameBgActive ]          = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.536000f );
        style.Colors[ ImGuiCol_TitleBg ]                = ImVec4( 0.000000f, 0.250000f, 0.500000f, 0.500000f );
        style.Colors[ ImGuiCol_TitleBgCollapsed ]       = ImVec4( 0.000000f, 0.000000f, 0.500000f, 0.500000f );
        style.Colors[ ImGuiCol_TitleBgActive ]          = ImVec4( 0.000000f, 0.500000f, 1.000000f, 0.800000f );
        style.Colors[ ImGuiCol_MenuBarBg ]              = ImVec4( 0.140000f, 0.140000f, 0.140000f, 1.000000f );
        style.Colors[ ImGuiCol_ScrollbarBg ]            = ImVec4( 0.020000f, 0.020000f, 0.020000f, 0.424000f );
        style.Colors[ ImGuiCol_ScrollbarGrab ]          = ImVec4( 0.310000f, 0.310000f, 0.310000f, 1.000000f );
        style.Colors[ ImGuiCol_ScrollbarGrabHovered ]   = ImVec4( 0.410000f, 0.410000f, 0.410000f, 1.000000f );
        style.Colors[ ImGuiCol_ScrollbarGrabActive ]    = ImVec4( 0.510000f, 0.510000f, 0.510000f, 1.000000f );
        style.Colors[ ImGuiCol_CheckMark ]              = ImVec4( 0.260000f, 0.590000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_SliderGrab ]             = ImVec4( 0.240000f, 0.520000f, 0.880000f, 1.000000f );
        style.Colors[ ImGuiCol_SliderGrabActive ]       = ImVec4( 0.260000f, 0.590000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_Button ]                 = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.320000f );
        style.Colors[ ImGuiCol_ButtonHovered ]          = ImVec4( 0.260000f, 0.590000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_ButtonActive ]           = ImVec4( 0.060000f, 0.530000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_Header ]                 = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.248000f );
        style.Colors[ ImGuiCol_HeaderHovered ]          = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.640000f );
        style.Colors[ ImGuiCol_HeaderActive ]           = ImVec4( 0.260000f, 0.590000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_ResizeGrip ]             = ImVec4( 0.000000f, 0.000000f, 0.000000f, 0.400000f );
        style.Colors[ ImGuiCol_ResizeGripHovered ]      = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.536000f );
        style.Colors[ ImGuiCol_ResizeGripActive ]       = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.760000f );
        style.Colors[ ImGuiCol_PlotLines ]              = ImVec4( 0.610000f, 0.610000f, 0.610000f, 1.000000f );
        style.Colors[ ImGuiCol_PlotLinesHovered ]       = ImVec4( 1.000000f, 0.430000f, 0.350000f, 1.000000f );
        style.Colors[ ImGuiCol_PlotHistogram ]          = ImVec4( 0.900000f, 0.700000f, 0.000000f, 1.000000f );
        style.Colors[ ImGuiCol_PlotHistogramHovered ]   = ImVec4( 1.000000f, 0.600000f, 0.000000f, 1.000000f );
        style.Colors[ ImGuiCol_TextSelectedBg ]         = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.280000f );
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GuiMgr::Term()
{
    m_pVB.Reset();
    m_pIB.Reset();
    m_pCB.Reset();
    m_pSmp.Reset();
    m_pTexture.Reset();
    m_pSRV.Reset();
    m_pRS.Reset();
    m_pBS.Reset();
    m_pDSS.Reset();
    m_pIL.Reset();
    m_pVS.Reset();
    m_pPS.Reset();
    m_pPSCube.Reset();
    m_pContext.Reset();
    m_pDevice.Reset();

    ImGui::DestroyContext();
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void GuiMgr::Update( uint32_t width, uint32_t height )
{
    auto time = std::chrono::system_clock::now();
    auto elapsedMilliSec = std::chrono::duration_cast<std::chrono::microseconds>( time - m_LastTime ).count();
    auto elapsedSec = static_cast<float>( double(elapsedMilliSec) / (1000.0 * 1000.0) );
    assert(elapsedSec > 0.0f); // ImGuiで落とされる前にチェックする.

    auto& io = ImGui::GetIO();
    io.DeltaTime     = elapsedSec;
    io.DisplaySize.x = float( width );
    io.DisplaySize.y = float( height );
    io.KeyCtrl       = ( GetKeyState( VK_CONTROL ) & 0x8000 ) != 0;
    io.KeyShift      = ( GetKeyState( VK_SHIFT )   & 0x8000 ) != 0;
    io.KeyAlt        = ( GetKeyState( VK_MENU )    & 0x8000 ) != 0;

    ImGui::NewFrame();

    m_LastTime = time;
}

//-----------------------------------------------------------------------------
//      描画処理です.
//-----------------------------------------------------------------------------
void GuiMgr::Draw()
{
    ImGui::Render();

    auto pDrawData = ImGui::GetDrawData();
    if ( uint32_t( pDrawData->TotalVtxCount ) >= m_SizeVB )
    {
        m_pVB.Reset();

        m_SizeVB = pDrawData->TotalVtxCount + 5000;

        D3D11_BUFFER_DESC desc = {};
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth      = m_SizeVB * sizeof( ImDrawVert );
        desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        auto hr = m_pDevice->CreateBuffer( &desc, nullptr, m_pVB.GetAddress() );
        if ( FAILED( hr ) )
        { return; }
    }

    if ( pDrawData->TotalIdxCount >= MaxPrimitiveCount * 6 )
    {
        m_pIB.Reset();

        m_SizeIB = pDrawData->TotalIdxCount + 10000;

        D3D11_BUFFER_DESC desc = {};
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth      = m_SizeIB * sizeof( ImDrawIdx );
        desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        auto hr = m_pDevice->CreateBuffer( &desc, nullptr, m_pIB.GetAddress() );
        if ( FAILED( hr ) )
        { return; }
    }

    D3D11_MAPPED_SUBRESOURCE resVB;
    D3D11_MAPPED_SUBRESOURCE resIB;
    auto ret = m_pContext->Map( m_pVB.GetPtr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resVB );
    if ( FAILED( ret ) )
    {
        m_pContext->Unmap( m_pVB.GetPtr(), 0 );
        return;
    }

    ret = m_pContext->Map( m_pIB.GetPtr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resIB );
    if ( FAILED( ret ) )
    {
        m_pContext->Unmap( m_pVB.GetPtr(), 0 );
        m_pContext->Unmap( m_pIB.GetPtr(), 0 );
        return;
    }

    auto pDstVtx = static_cast<ImDrawVert*>( resVB.pData );
    auto pDstIdx = static_cast<ImDrawIdx*> ( resIB.pData );

    for ( auto i = 0; i < pDrawData->CmdListsCount; ++i )
    {
        const auto pCmdList = pDrawData->CmdLists[ i ];
        memcpy( pDstVtx, pCmdList->VtxBuffer.Data, pCmdList->VtxBuffer.size() * sizeof( ImDrawVert ) );
        memcpy( pDstIdx, pCmdList->IdxBuffer.Data, pCmdList->IdxBuffer.size() * sizeof( ImDrawIdx ) );
        pDstVtx += pCmdList->VtxBuffer.size();
        pDstIdx += pCmdList->IdxBuffer.size();
    }

    m_pContext->Unmap( m_pVB.GetPtr(), 0 );
    m_pContext->Unmap( m_pIB.GetPtr(), 0 );

    {
        float L = 0.0f;
        float R = ImGui::GetIO().DisplaySize.x;
        float B = ImGui::GetIO().DisplaySize.y;
        float T = 0.0f;

        float mvp[ 4 ][ 4 ] = {
            { 2.0f / ( R - L ),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f / ( T - B ),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { ( R + L ) / ( L - R ),  ( T + B ) / ( B - T ),    0.5f,       1.0f },
        };

        m_pContext->UpdateSubresource( m_pCB.GetPtr(), 0, nullptr, mvp, 0, 0 );
    }

    {
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX   = 0.0f;
        viewport.TopLeftY   = 0.0f;
        viewport.Width      = ImGui::GetIO().DisplaySize.x;
        viewport.Height     = ImGui::GetIO().DisplaySize.y;
        viewport.MinDepth   = 0.0f;
        viewport.MaxDepth   = 1.0f;

        m_pContext->RSSetViewports( 1, &viewport );
    }

    {
        uint32_t stride = sizeof( ImDrawVert );
        uint32_t offset = 0;

        m_pContext->IASetInputLayout( m_pIL.GetPtr() );
        m_pContext->IASetVertexBuffers( 0, 1, m_pVB.GetAddress(), &stride, &offset );
        m_pContext->IASetIndexBuffer( m_pIB.GetPtr(), sizeof( ImDrawIdx ) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0 );
        m_pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        m_pContext->VSSetShader( m_pVS.GetPtr(), nullptr, 0 );
        m_pContext->VSSetConstantBuffers( 0, 1, m_pCB.GetAddress() );
        m_pContext->PSSetShader( m_pPS.GetPtr(), nullptr, 0 );
        m_pContext->PSSetSamplers( 0, 1, m_pSmp.GetAddress() );

        const float blend_factor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_pContext->OMSetBlendState( m_pBS.GetPtr(), blend_factor, 0xffffffff );
        m_pContext->OMSetDepthStencilState( m_pDSS.GetPtr(), 0 );
        m_pContext->RSSetState( m_pRS.GetPtr() );
    }

    {
        int offsetVtx = 0;
        int offsetIdx = 0;
        auto changeTexture = false;

        for ( auto i = 0; i < pDrawData->CmdListsCount; ++i )
        {
            const auto pCmdList = pDrawData->CmdLists[ i ];
            for ( auto j = 0; j < pCmdList->CmdBuffer.size(); ++j )
            {
                const auto pCmd = &pCmdList->CmdBuffer[ j ];
                if ( pCmd->UserCallback )
                {
                    pCmd->UserCallback( pCmdList, pCmd );
                }
                else
                {
                    // テクスチャが渡された場合は変更.
                    if (pCmd->TextureId != nullptr)
                    {
                        auto pSRV = reinterpret_cast<ID3D11ShaderResourceView*>(pCmd->TextureId);
                        changeTexture = true;

                        ID3D11Texture2D* pRes;
                        pSRV->GetResource(reinterpret_cast<ID3D11Resource**>(&pRes));

                        D3D11_TEXTURE2D_DESC desc;
                        pRes->GetDesc(&desc);

                        if (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
                        { m_pContext->PSSetShader(m_pPSCube.GetPtr(), nullptr, 0); }
                        else
                        { m_pContext->PSSetShader(m_pPS.GetPtr(), nullptr, 0); }

                        m_pContext->PSSetShaderResources(0, 1, &pSRV);

                        pRes->Release();
                    }
                    else
                    {
                        // フォントのテクスチャに戻す.
                        if (changeTexture)
                        {
                            m_pContext->PSSetShader(m_pPS.GetPtr(), nullptr, 0);
                            m_pContext->PSSetShaderResources(0, 1, m_pSRV.GetAddress());
                            changeTexture = false;
                        }
                    }

                    const D3D11_RECT rc = {
                        LONG( pCmd->ClipRect.x ),
                        LONG( pCmd->ClipRect.y ),
                        LONG( pCmd->ClipRect.z ),
                        LONG( pCmd->ClipRect.w )
                    };

                    m_pContext->RSSetScissorRects( 1, &rc );
                    m_pContext->DrawIndexed( pCmd->ElemCount, offsetIdx, offsetVtx );
                }
                offsetIdx += pCmd->ElemCount;
            }
            offsetVtx += pCmdList->VtxBuffer.size();
        }
    }

    {
        ID3D11SamplerState*       pNullSmp[] = { nullptr };
        ID3D11ShaderResourceView* pNullSRV[] = { nullptr };

        m_pContext->PSSetSamplers( 0, 1, pNullSmp );
        m_pContext->PSSetShaderResources( 0, 1, pNullSRV );
        m_pContext->VSSetShader( nullptr, nullptr, 0 );
        m_pContext->PSSetShader( nullptr, nullptr, 0 );
    }
}

//-----------------------------------------------------------------------------
//      マウスの処理です.
//-----------------------------------------------------------------------------
void GuiMgr::OnMouse( int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR )
{
    auto& io = ImGui::GetIO();

    io.MousePosPrev = io.MousePos;
    io.MousePos = ImVec2( float( x ), float( y ) );
    io.MouseDown[ 0 ] = isDownL;
    io.MouseDown[ 1 ] = isDownR;
    io.MouseDown[ 2 ] = isDownM;
    io.MouseDown[ 3 ] = false;
    io.MouseDown[ 4 ] = false;
    if ( wheelDelta > 0 )
    {
        io.MouseWheel = 1.0f;
    }
    else if ( wheelDelta < 0 )
    {
        io.MouseWheel = -1.0f;
    }
    else
    {
        io.MouseWheel = 0;
    }
}

//-----------------------------------------------------------------------------
//      キーの処理です.
//-----------------------------------------------------------------------------
void GuiMgr::OnKey( bool isDown, bool isAltDown, uint32_t code )
{
    auto& io = ImGui::GetIO();

    io.KeysDown[ code ] = isDown;
    io.KeyAlt = isAltDown;
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void GuiMgr::OnTyping( uint32_t code )
{
    if ( code > 0 && code < 0x10000 )
    {
        auto& io = ImGui::GetIO();
        io.AddInputCharacter( ImWchar( code ) );
    }
}

} // namespace asdx

#endif//ASDX_ENABLE_IMGUI
