#include "stdafx.h"

#include "RenderWindow.h"

#include "ShaderCache.h"
#include <Math/Vector.h>
#include <System/Window.h>
#include <algorithm>

#if SG_PLATFORM_IS_WIN
#include <dxgi.h>
#include <d3d11.h>
#include <Core/WindowsH.h>
#endif

namespace sg {
namespace rendering {
//=============================================================================
RenderWindow::RenderWindow(RenderDevice* iRenderDevice, system::Window* iWindow)
    : m_renderDevice(iRenderDevice)
    , m_window(iWindow)
    , m_swapChain()
    , m_backBuffer()
    , m_backBufferRTV()
    , m_backBufferAsLinearRGBImpl(this)
{
    uint2 wh = m_window->ClientSize().Get();

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_window->hWnd();
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    ID3D11Device* d3dDevice = m_renderDevice->D3DDevice();
    // TODO: Migrate on IDXGIFactory2
    comptr<IDXGIDevice1> dxgiDevice;
    HRESULT hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)dxgiDevice.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));
    comptr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)dxgiAdapter.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));
    comptr<IDXGIFactory1> dxgiFactory;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), (void**)dxgiFactory.GetPointerForInitialisation());

    hr = dxgiFactory->CreateSwapChain(
        d3dDevice,
        &sd,
        m_swapChain.GetPointerForInitialisation()
        );
    SG_ASSERT(SUCCEEDED(hr));

    /*
    hr = dxgiFactory->CreateSwapChainForHwnd(
        m_d3dDevice.get(),
        m_windowAccess->hwnd,
        &sd,
        NULL,
        NULL,
        m_swapChain.GetPointerForInitialisation()
        );
    SG_ASSERT(SUCCEEDED(hr));
        */

    m_window->ClientSize().RegisterObserver(this);
    m_renderDevice->GetPresentFrameNotification().RegisterObserver(this);

    m_renderTargetResolution.Set(m_window->ClientSize().Get());
    m_depthStencilSurface = new DepthStencilSurface(m_renderDevice.get(), &m_renderTargetResolution);

    UpdateBackBuffer();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RenderWindow::~RenderWindow()
{
    m_renderDevice->GetPresentFrameNotification().UnregisterObserver(this);
    m_window->ClientSize().UnregisterObserver(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderWindow::VirtualOnNotified(ObservableValue<uint2> const* iValue)
{
    SG_ASSERT(iValue == &(m_window->ClientSize()));
    SG_ASSERT(!m_renderDevice->IsInRender_ForAssert());
    uint2 const wh = iValue->Get();
    m_renderTargetResolution.Set(wh);

    // cf. http://msdn.microsoft.com/en-us/library/windows/desktop/bb205075%28v=vs.85%29.aspx#Handling_Window_Resizing

    // Release all outstanding references to the swap chain's buffers.
    m_backBufferAsLinearRGBRTV = nullptr;
    m_backBufferRTV = nullptr;
    m_backBuffer = nullptr;

    HRESULT hr;
    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    SG_ASSERT(SUCCEEDED(hr));

    UpdateBackBuffer();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderWindow::UpdateBackBuffer()
{
    HRESULT hr;
    hr = m_swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ),
                                ( LPVOID* )m_backBuffer.GetPointerForInitialisation() );
    SG_ASSERT(SUCCEEDED(hr));

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    hr = m_renderDevice->D3DDevice()->CreateRenderTargetView(m_backBuffer.get(), NULL,
                                                             m_backBufferRTV.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    hr = m_renderDevice->D3DDevice()->CreateRenderTargetView(m_backBuffer.get(), &rtvDesc,
                                                             m_backBufferAsLinearRGBRTV.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RenderWindow::VirtualOnNotified(PresentFrameNotification const* iPresentFrame)
{
    SG_UNUSED((iPresentFrame));
    m_swapChain->Present(1, 0);
}
//=============================================================================
}
}

