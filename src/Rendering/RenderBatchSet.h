#ifndef Rendering_RenderBatchSet_H
#define Rendering_RenderBatchSet_H

#include <Core/SmartPtr.h>
#include <unordered_set>

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
    void Insert(rendering::IRenderBatch* iRenderBatch);
    void Remove(rendering::IRenderBatch* iRenderBatch);
    void Execute(RenderDevice const* iRenderDevice,
                 IShaderConstantDatabase const* iShaderConstantDatabase,
                 IShaderResourceDatabase const* iShaderResourceDatabase);
private:
    std::unordered_set<safeptr<rendering::IRenderBatch> > m_renderBatches;
    bool m_needPrioritySort;
};
//=============================================================================
}
}

#endif
