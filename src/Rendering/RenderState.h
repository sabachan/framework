#ifndef Rendering_RenderState_H
#define Rendering_RenderState_H

#include <Core/FilePath.h>
#include <Core/SmartPtr.h>
#include <Core/ArrayView.h>

#include <d3d11.h>

struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;

namespace sg {
namespace rendering {
//=============================================================================
class RenderStateDico;
//=============================================================================
class BlendStateDescriptor : public RefAndSafeCountable
{
public:
    BlendStateDescriptor() {}
    size_t Hash() const;
    bool Equals(BlendStateDescriptor const& b) const;
    D3D11_BLEND_DESC& GetWritableDesc() { return desc; }
    D3D11_BLEND_DESC const& GetDesc() const { return desc; }
private:
    // TODO: change for a multiplatform description
    D3D11_BLEND_DESC desc;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class DepthStencilDescriptor : public RefAndSafeCountable
{
public:
    DepthStencilDescriptor() {}
    size_t Hash() const;
    bool Equals(DepthStencilDescriptor const& b) const;
    D3D11_DEPTH_STENCIL_DESC& GetWritableDesc() { return desc; }
private:
    // TODO: change for a multiplatform description
    D3D11_DEPTH_STENCIL_DESC desc;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class RasterizerStateDescriptor : public RefAndSafeCountable
{
public:
    RasterizerStateDescriptor() {}
    size_t Hash() const;
    bool Equals(RasterizerStateDescriptor const& b) const;
    D3D11_RASTERIZER_DESC& GetWritableDesc() { return desc; }
private:
    // TODO: change for a multiplatform description
    D3D11_RASTERIZER_DESC desc;
};
//=============================================================================
}
}

#endif
