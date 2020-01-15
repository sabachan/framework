#ifndef Rendering_RenderBatchPassId_H
#define Rendering_RenderBatchPassId_H

namespace sg {
namespace rendering {
//=============================================================================
struct RenderBatchPassId
{
    RenderBatchPassId() = default;
    explicit RenderBatchPassId(int iIndex) : index(iIndex) {}
    int index = 0;

    friend bool operator == (RenderBatchPassId a, RenderBatchPassId b) { return a.index == b.index; }
    friend bool operator != (RenderBatchPassId a, RenderBatchPassId b) { return a.index != b.index; }
};
//=============================================================================
}
}

#endif
