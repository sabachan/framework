#ifndef Rendering_RenderDevice_H
#define Rendering_RenderDevice_H

#include "IRenderTarget.h"
#include "RenderStateDico.h"
#include "ResolutionServer.h"
#include <Core/ComPtr.h>
#include <Core/Observer.h>
#include <Core/Singleton.h>
#include <Core/SmartPtr.h>
#include <Math/Vector.h>

struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11Device;
struct ID3D11DeviceChild;
struct ID3D11DeviceContext;
struct ID3D11RasterizerState;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct IDXGISwapChain;

namespace sg {
//=============================================================================
namespace rendering {
//=============================================================================
class PresentFrameNotification : public Observable<PresentFrameNotification> {};
//=============================================================================
#if SG_ENABLE_ASSERT
void SetDebugName(ID3D11DeviceChild* child, std::string const& name);
#endif
//=============================================================================
class RenderDevice : public Singleton<RenderDevice>
{
public:
    RenderDevice();
    ~RenderDevice();

    void SetDefaultState() const;
    void BeginRender();
    void EndRender();
    ID3D11Device* D3DDevice() const { return m_d3dDevice.get(); }
    ID3D11DeviceContext* ImmediateContext() const { return m_immediateContext.get(); }
    PresentFrameNotification& GetPresentFrameNotification() { return m_presentFrame; }
    ID3D11BlendState* DefaultBlendState() const { return m_blendState.get(); }
#if SG_ENABLE_ASSERT
    bool IsInRender_ForAssert() { return m_isInRender; }
#endif
    RenderStateName RenderStateName_NoBlend() const { return m_noBlendName; }
    RenderStateName RenderStateName_PremultipliedAlphaBlending() const { return m_premultipliedAlphaBlendingName; }
private:
    comptr<ID3D11Device> m_d3dDevice;
    comptr<ID3D11DeviceContext> m_immediateContext;

    //comptr<ID3D11Texture2D> m_backBuffer;
    //comptr<ID3D11RenderTargetView> m_backBufferRTV;
    comptr<ID3D11RasterizerState> m_rasterizerState;
    comptr<ID3D11BlendState> m_blendState;

    //refptr<DepthStencilSurface> m_depthStencilSurface;
    comptr<ID3D11DepthStencilState> m_depthStencilState;

    PresentFrameNotification m_presentFrame;

    RenderStateName m_noBlendName;
    RenderStateName m_premultipliedAlphaBlendingName;
#if SG_ENABLE_ASSERT
    bool m_isInRender;
#endif
};
//=============================================================================
}
}

#endif
