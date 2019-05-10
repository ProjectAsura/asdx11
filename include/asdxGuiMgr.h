//-------------------------------------------------------------------------------------------------
// File : GuiMgr.h
// Desc : Gui Manager.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

#ifdef ASDX_ENABLE_IMGUI

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>
#include <chrono>
#include <d3d11.h>
#include <asdxRef.h>


//-------------------------------------------------------------------------------------------------
// Forward Declarations.
//-------------------------------------------------------------------------------------------------
struct ImDrawData;


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// GuiMgr class
///////////////////////////////////////////////////////////////////////////////////////////////////
class GuiMgr
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

    //---------------------------------------------------------------------------------------------
    // Static
    //---------------------------------------------------------------------------------------------
    static constexpr uint32_t   MaxPrimitiveCount = 6 * 1024;
    static GuiMgr               s_Instance;

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    static GuiMgr& GetInstance();
    bool Init( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd, uint32_t width, uint32_t height );
    void Term();
    void Update( uint32_t width, uint32_t height );
    void Draw();
    void OnDraw( ImDrawData* draw_data );
    void OnMouse( int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR );
    void OnKey( bool isDown, bool isAltDown, uint32_t code );
    void OnTyping( uint32_t code );

private:
    //=============================================================================================
    // private varaibles.
    //=============================================================================================
    RefPtr<ID3D11Device>                    m_pDevice;
    RefPtr<ID3D11DeviceContext>             m_pContext;
    RefPtr<ID3D11Buffer>                    m_pVB;
    RefPtr<ID3D11Buffer>                    m_pIB;
    RefPtr<ID3D11Buffer>                    m_pCB;
    RefPtr<ID3D11SamplerState>              m_pSmp;
    RefPtr<ID3D11Texture2D>                 m_pTexture;
    RefPtr<ID3D11ShaderResourceView>        m_pSRV;
    RefPtr<ID3D11RasterizerState>           m_pRS;
    RefPtr<ID3D11BlendState>                m_pBS;
    RefPtr<ID3D11DepthStencilState>         m_pDSS;
    RefPtr<ID3D11InputLayout>               m_pIL;
    RefPtr<ID3D11VertexShader>              m_pVS;
    RefPtr<ID3D11PixelShader>               m_pPS;
    std::chrono::system_clock::time_point   m_LastTime;
    uint32_t                                m_SizeVB;
    uint32_t                                m_SizeIB;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    GuiMgr();
    ~GuiMgr();
};

} // namespace asdx

#endif//ASDX_ENABLE_IMGUI