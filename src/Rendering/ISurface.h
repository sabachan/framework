#ifndef Rendering_ISurface_H
#define Rendering_ISurface_H

#include "SurfaceFormat.h"
#include <Core/SmartPtr.h>

struct ID3D11Texture2D;

namespace sg {
namespace rendering {
//=============================================================================
class ResolutionServer;
//=============================================================================
class ISurface : public VirtualRefAndSafeCountable
{
public:
    virtual ~ISurface() override {}
    virtual ID3D11Texture2D* GetSurfaceTexture() const = 0;
    virtual ResolutionServer const* GetSurfaceResolution() const = 0;
    virtual SurfaceFormat GetSurfaceFormat() const = 0;
};
//=============================================================================
}
}

#endif
