//-----------------------------------------------------------------------------
// File : SampleApp.h
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxApp.h>


///////////////////////////////////////////////////////////////////////////////
// SampleApp class
///////////////////////////////////////////////////////////////////////////////
class SampleApp : public asdx::Application
{
public:
    SampleApp();
    ~SampleApp();

private:
    bool OnInit() override;
    void OnTerm() override;
    void OnFrameRender() override;
    void OnResize(const asdx::ResizeEventArgs& param) override;
    void OnKey(const asdx::KeyEventArgs& param) override;
    void OnMouse(const asdx::MouseEventArgs& param) override;
    void OnTyping(uint32_t keyCode) override;
};