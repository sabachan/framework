#ifndef Rendering_RenderWindow_H
#define Rendering_RenderWindow_H

#include <Core/ComPtr.h>
#include <Core/Observer.h>
#include <Core/SmartPtr.h>
#include <Math/Vector.h>
#include "Surface.h"
#include "RenderDevice.h"
#include "RenderTarget.h"
#include "ResolutionServer.h"

struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct IDXGISwapChain;

namespace sg {
namespace system {
class Window;
}
}

namespace sg {
namespace rendering {
//=============================================================================
class RenderWindow : public RefCountable
                   , public IRenderTarget
                   , private Observer<ObservableValue<uint2> >
                   , private Observer<PresentFrameNotification>
{
    PARENT_REF_COUNTABLE(RefCountable)
    PARENT_SAFE_COUNTABLE(Observer<ObservableValue<uint2> >)
public:
    RenderWindow(RenderDevice* iRenderDevice, system::Window* iWindow);
    virtual ~RenderWindow() override;

    virtual ID3D11RenderTargetView* GetRenderTargetView() const override { return m_backBufferRTV.get(); }
    virtual ResolutionServer const* RenderTargetResolution() const override { return &m_renderTargetResolution; }

    IRenderTarget* BackBuffer() { return this; }
    IRenderTarget* BackBufferAsLinearRGB() { return &m_backBufferAsLinearRGBImpl; }
    DepthStencilSurface* GetDepthStencilSurface() const { return m_depthStencilSurface.get(); }
private:
    virtual void VirtualOnNotified(ObservableValue<uint2> const* iValue) override;
    virtual void VirtualOnNotified(PresentFrameNotification const* iPresentFrame) override;
    void UpdateBackBuffer();
private:
    struct BackBufferAsLinearRGBImpl : public IRenderTarget
    {
        BackBufferAsLinearRGBImpl(RenderWindow* iRenderWidow) : m_renderWindow(iRenderWidow) {}
        virtual RefCountable const* VirtualGetAsRefCountable() const override { return m_renderWindow.get(); }
        virtual ID3D11RenderTargetView* GetRenderTargetView() const override { return m_renderWindow->m_backBufferAsLinearRGBRTV.get(); }
        virtual ResolutionServer const* RenderTargetResolution() const override { return &m_renderWindow->m_renderTargetResolution; }
        safeptr<RenderWindow> m_renderWindow;
    };
    safeptr<RenderDevice> m_renderDevice;
    safeptr<system::Window> m_window;
    ResolutionServer m_renderTargetResolution;

    comptr<IDXGISwapChain> m_swapChain;
    comptr<ID3D11Texture2D> m_backBuffer;
    comptr<ID3D11RenderTargetView> m_backBufferRTV;
    comptr<ID3D11RenderTargetView> m_backBufferAsLinearRGBRTV;
    BackBufferAsLinearRGBImpl m_backBufferAsLinearRGBImpl;

    refptr<DepthStencilSurface> m_depthStencilSurface;
};
//=============================================================================
}
}

#endif
