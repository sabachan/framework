#ifndef Rendering_ResolutionServer_H
#define Rendering_ResolutionServer_H

#include <Core/Observer.h>
#include <Math/Vector.h>

namespace sg {
namespace rendering {
//=============================================================================
class ResolutionServer : public ObservableValueHelper<ResolutionServer, uint2>
{
public:
    ResolutionServer() {}
    ResolutionServer(uint2 const& iValue) : ObservableValueHelper(iValue) {}
};
//=============================================================================
}
}

#endif
