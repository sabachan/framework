#include "stdafx.h"

#include "RenderState.h"

#include "WTF/IncludeD3D11.h"

namespace sg {
namespace rendering {
//=============================================================================
size_t BlendStateDescriptor::Hash() const
{
#define BIT_ROTATE(A, N) do { A = (A << N) ^ (A >> (8*sizeof(h)-N)); } while(SG_CONSTANT_CONDITION(0))
    size_t h = (desc.AlphaToCoverageEnable ? 2 : 0) | (desc.IndependentBlendEnable ? 1 : 0);
    size_t const renderTargetCount = desc.IndependentBlendEnable ? 8 : 1;
    for(size_t i = 0; i < renderTargetCount; ++i)
    {
        BIT_ROTATE(h, 3);
        h ^= desc.RenderTarget[i].BlendEnable ? 1 : 0;
        BIT_ROTATE(h, 5);
        h ^= static_cast<size_t>(desc.RenderTarget[i].SrcBlend);
        BIT_ROTATE(h, 5);
        h ^= static_cast<size_t>(desc.RenderTarget[i].DestBlend);
        BIT_ROTATE(h, 3);
        h ^= static_cast<size_t>(desc.RenderTarget[i].BlendOp);
        BIT_ROTATE(h, 5);
        h ^= static_cast<size_t>(desc.RenderTarget[i].SrcBlendAlpha);
        BIT_ROTATE(h, 5);
        h ^= static_cast<size_t>(desc.RenderTarget[i].DestBlendAlpha);
        BIT_ROTATE(h, 3);
        h ^= static_cast<size_t>(desc.RenderTarget[i].BlendOpAlpha);
        BIT_ROTATE(h, 8);
        h ^= desc.RenderTarget[i].RenderTargetWriteMask;
    }
#undef BIT_ROTATE
    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool BlendStateDescriptor::Equals(BlendStateDescriptor const& b) const
{
    if(desc.AlphaToCoverageEnable != b.desc.AlphaToCoverageEnable)
        return false;
    if(desc.IndependentBlendEnable != b.desc.IndependentBlendEnable)
        return false;
    size_t const renderTargetCount = desc.IndependentBlendEnable ? 8 : 1;
    for(size_t i = 0; i < renderTargetCount; ++i)
    {
        if(desc.RenderTarget[i].BlendEnable != b.desc.RenderTarget[i].BlendEnable)
            return false;
        if(desc.RenderTarget[i].SrcBlend != b.desc.RenderTarget[i].SrcBlend)
            return false;
        if(desc.RenderTarget[i].DestBlend != b.desc.RenderTarget[i].DestBlend)
            return false;
        if(desc.RenderTarget[i].BlendOp != b.desc.RenderTarget[i].BlendOp)
            return false;
        if(desc.RenderTarget[i].SrcBlendAlpha != b.desc.RenderTarget[i].SrcBlendAlpha)
            return false;
        if(desc.RenderTarget[i].DestBlendAlpha != b.desc.RenderTarget[i].DestBlendAlpha)
            return false;
        if(desc.RenderTarget[i].BlendOpAlpha != b.desc.RenderTarget[i].BlendOpAlpha)
            return false;
        if(desc.RenderTarget[i].RenderTargetWriteMask != b.desc.RenderTarget[i].RenderTargetWriteMask)
            return false;
    }
    return true;
}
//=============================================================================
}
}
