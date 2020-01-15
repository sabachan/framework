#ifndef Rendering_RenderStateDico_H
#define Rendering_RenderStateDico_H

#include "RenderState.h"

#include <Core/IntTypes.h>
#include <Core/FastSymbol.h>
#include <Core/ArrayView.h>

struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;

namespace sg {
namespace rendering {
class RenderDevice;
//=============================================================================
FAST_SYMBOL_TYPE_HEADER(RenderStateName)
//=============================================================================
namespace renderstatedico {
//=============================================================================
void Init();
void SetRenderDevice(RenderDevice* iRenderDevice);
ID3D11BlendState* GetBlendState(RenderStateName iName, BlendStateDescriptor const* iDescIfNeedCreation = nullptr);
ID3D11DepthStencilState* GetDepthStencilState(RenderStateName iName, DepthStencilDescriptor const* iDescIfNeedCreation = nullptr);
ID3D11RasterizerState* GetRasterizerState(RenderStateName iName, RasterizerStateDescriptor const* iDescIfNeedCreation = nullptr);
void Shutdown();
//=============================================================================
}
}
}

#endif
