#ifndef Rendering_RenderBatch_H
#define Rendering_RenderBatch_H

#include <Core/SmartPtr.h>
#include "RenderBatchPassId.h"

namespace sg {
namespace rendering {
//=============================================================================
class IShaderConstantDatabase;
class IShaderResourceDatabase;
class RenderBatchSet;
class RenderDevice;
//=============================================================================
// This interface should be derived by anyone needing to render. It is used for
// correctly ordering the draw calls.
// The pass id is used for a render batch to generate draw calls in multiple
// layers (eg. z-prepass, opaque and shadow).
//=============================================================================
class IRenderBatch : public RefAndSafeCountable
{
public:
    // The priority is used to sort batches. Highest priority is display after,
    // hence seems to be on top of other batches.
    // This can be used when one drawer needs to render two or more materials
    // in a specified order. However, most of the time, describing compositing
    // order externally would be prefered.
    // One exemple of Priority usage is for drawing strokes on text, where text
    // itself can use different materials for different glyphs, and stroke is
    // so large that it would overwrite other glyphs if rendered with the same
    // shader as the fill glyph shader. Having two passes, first one for all
    // the stroke materials, second one for all the fill glyph materials
    // provides correct rendering (except that strokes overlap, but it's OK as
    // long as it is opaque)
    virtual int GetPriority(RenderBatchPassId iPassId) = 0;
    virtual void PreExecute(RenderBatchPassId iPassId,
                            RenderDevice const* iRenderDevice,
                            IShaderConstantDatabase const* iShaderConstantDatabase,
                            IShaderResourceDatabase const* iShaderResourceDatabase) = 0;
    virtual size_t GetSubLayerEnd(RenderBatchPassId iPassId) = 0;
    // ioNextSubLayer can be used to specified when the batch has to be
    // executed again. this is interesting when layer indices are sparse, in
    // order not to be called for each intermediate index.
    virtual void Execute(RenderBatchPassId iPassId,
                         RenderDevice const* iRenderDevice,
                         IShaderConstantDatabase const* iShaderConstantDatabase,
                         IShaderResourceDatabase const* iShaderResourceDatabase,
                         size_t iSubLayer,
                         size_t& ioNextSubLayer) = 0;
    virtual void PostExecute(RenderBatchPassId iPassId) = 0;
#if SG_ENABLE_ASSERT
public:
    IRenderBatch();
    virtual ~IRenderBatch();
private:
    friend class RenderBatchSet;
    safeptr<RenderBatchSet> m_setWhereRegistered;
#else
public:
    virtual ~IRenderBatch() {}
#endif
};
//=============================================================================

//=============================================================================
}
}

#endif
