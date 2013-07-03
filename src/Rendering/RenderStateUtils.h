#ifndef Rendering_RenderStateUtils_H
#define Rendering_RenderStateUtils_H

#include <Core/ComPtr.h>
#include <d3d11.h>

namespace sg {
namespace rendering {
//=============================================================================
class RenderDevice;
//=============================================================================
class SaveRenderStateRestoreAtEnd
{
public:
    SaveRenderStateRestoreAtEnd(RenderDevice const* iRenderDevice);
    ~SaveRenderStateRestoreAtEnd();
private:
    safeptr<RenderDevice const> m_renderDevice;
    comptr<ID3D11BlendState> blendState;
    FLOAT blendFactor[4];
    UINT sampleMask;
    D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    UINT viewportCount;
    D3D11_RECT scissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    UINT scissorRectCount;
    ID3D11RenderTargetView* renderTargetViewArray[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
    comptr<ID3D11RenderTargetView> renderTargetView[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
    UINT renderTargetViewCount;
    comptr<ID3D11DepthStencilView> depthStencilView;
};
//=============================================================================
}
}

#endif
