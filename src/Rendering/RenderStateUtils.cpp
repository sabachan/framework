#include "stdafx.h"

#include "RenderStateUtils.h"

#include "RenderDevice.h"

namespace sg {
namespace rendering {
//=============================================================================
SaveRenderStateRestoreAtEnd::SaveRenderStateRestoreAtEnd(RenderDevice const* iRenderDevice)
: m_renderDevice(iRenderDevice)
{
    ID3D11DeviceContext* context = m_renderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    context->OMGetBlendState( blendState.GetPointerForInitialisation(), blendFactor, &sampleMask );

    viewportCount = SG_ARRAYSIZE(viewports);
    context->RSGetViewports(&viewportCount, viewports);

    scissorRectCount = SG_ARRAYSIZE(scissorRects);
    context->RSGetScissorRects(&scissorRectCount, scissorRects);

    renderTargetViewCount = SG_ARRAYSIZE(renderTargetViewArray);
    context->OMGetRenderTargets(renderTargetViewCount, renderTargetViewArray, depthStencilView.GetPointerForInitialisation());
    for(size_t i = 0; i < renderTargetViewCount; ++i)
    {
        *renderTargetView[i].GetPointerForInitialisation() = renderTargetViewArray[i];
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SaveRenderStateRestoreAtEnd::~SaveRenderStateRestoreAtEnd()
{
    ID3D11DeviceContext* context = m_renderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    context->OMSetBlendState(blendState.get(), blendFactor, sampleMask);
    context->RSSetViewports(viewportCount, viewports);
    context->RSSetScissorRects(scissorRectCount, scissorRects);
    context->OMSetRenderTargets(SG_ARRAYSIZE(renderTargetViewArray), renderTargetViewArray, depthStencilView.get());
}
//=============================================================================
}
}
