#ifndef Rendering_RenderBatchSet_H
#define Rendering_RenderBatchSet_H

#include <Core/HashMap.h>
#include <Core/SmartPtr.h>
#include "RenderBatchPassId.h"

namespace sg {
namespace rendering {
//=============================================================================
class IShaderConstantDatabase;
class IShaderResourceDatabase;
class RenderDevice;
class IRenderBatch;
//=============================================================================
// A RenderBatchSet contains a set of render batches and is able to draw them.
class RenderBatchSet : public SafeCountable
{
public:
    RenderBatchSet();
    ~RenderBatchSet();
    void Insert(IRenderBatch* iRenderBatch, RenderBatchPassId iPassId = RenderBatchPassId());
    void Remove(IRenderBatch* iRenderBatch, RenderBatchPassId iPassId = RenderBatchPassId());
    void Execute(RenderDevice const* iRenderDevice,
                 IShaderConstantDatabase const* iShaderConstantDatabase,
                 IShaderResourceDatabase const* iShaderResourceDatabase);
private:
    struct RenderBatchPass
    {
        safeptr<IRenderBatch> batch;
        RenderBatchPassId pass;

        struct HasherComparer
        {
            size_t operator()(RenderBatchPass const& a) const { return size_t(a.batch.get()) + a.pass.index; }
            bool operator()(RenderBatchPass const& a, RenderBatchPass const& b) const { return a.batch == b.batch && a.pass == b.pass; }
        };
    };
    HashSet<RenderBatchPass, RenderBatchPass::HasherComparer, RenderBatchPass::HasherComparer> m_renderBatches;
    bool m_needPrioritySort;
};
//=============================================================================
}
}

#endif
