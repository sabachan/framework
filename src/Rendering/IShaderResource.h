#ifndef Rendering_IShaderResource_H
#define Rendering_IShaderResource_H

#include <Core/SmartPtr.h>

struct ID3D11ShaderResourceView;

namespace sg {
namespace rendering {
//=============================================================================
class ResolutionServer;
//=============================================================================
class IShaderResource : public VirtualRefAndSafeCountable
{
public:
    virtual ~IShaderResource() override {}
    virtual ID3D11ShaderResourceView* GetShaderResourceView() const = 0;
    virtual ResolutionServer const* ShaderResourceResolution() const = 0;
};
//=============================================================================
}
}

#endif
