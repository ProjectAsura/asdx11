//-----------------------------------------------------------------------------
// File : SampleApp.cpp
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SampleApp.h>


///////////////////////////////////////////////////////////////////////////////
// SampleApp class
///////////////////////////////////////////////////////////////////////////////
SampleApp::SampleApp()
{
    m_MultiSampleCount      = 1;
    m_MultiSampleQuality    = 0;
    m_SwapChainFormat       = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_DepthStencilFormat    = DXGI_FORMAT_D32_FLOAT;
}

SampleApp::~SampleApp()
{
}

bool SampleApp::OnInit()
{
    return true;
}

void SampleApp::OnTerm()
{
}

void SampleApp::OnFrameRender()
{
    auto pRTV = m_ColorTarget2D.GetTargetView();
    auto pDSV = m_DepthTarget2D.GetTargetView();

    if (pRTV == nullptr)
    { return; }

    m_pDeviceContext->OMSetRenderTargets(1, &pRTV, pDSV);

    m_pDeviceContext->ClearRenderTargetView(pRTV, m_ClearColor);
    m_pDeviceContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


    Present(0);
}

void SampleApp::OnResize(const asdx::ResizeEventArgs& param)
{
}

void SampleApp::OnKey(const asdx::KeyEventArgs& param)
{
}

void SampleApp::OnMouse(const asdx::MouseEventArgs& param)
{
}

void SampleApp::OnTyping(uint32_t keyCode)
{
}