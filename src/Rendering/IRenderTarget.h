#ifndef Rendering_IRenderTarget_H
#define Rendering_IRenderTarget_H

#include <Core/SmartPtr.h>

struct ID3D11RenderTargetView;

namespace sg {
namespace rendering {
//=============================================================================
class ResolutionServer;
//=============================================================================
class IRenderTarget : public VirtualRefAndSafeCountable
{
public:
    virtual ~IRenderTarget() override {}
    virtual ID3D11RenderTargetView* GetRenderTargetView() const = 0;
    virtual ResolutionServer const* RenderTargetResolution() const = 0;
};
//=============================================================================
}
}

#endif
