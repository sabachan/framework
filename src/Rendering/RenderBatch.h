#ifndef Rendering_RenderBatch_H
#define Rendering_RenderBatch_H

#include <Core/SmartPtr.h>

namespace sg {
namespace rendering {
//=============================================================================
class IShaderConstantDatabase;
class IShaderResourceDatabase;
class RenderBatchSet;
class RenderDevice;
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
    virtual int GetPriority() = 0;
    virtual void PreExecute(RenderDevice const* iRenderDevice,
                            IShaderConstantDatabase const* iShaderConstantDatabase,
                            IShaderResourceDatabase const* iShaderResourceDatabase) = 0;
    virtual size_t GetSubLayerCount() = 0;
    // ioNextSubLayer can be used to specified when the batch has to be
    // executed again. this is interesting when layer indices are sparse, in
    // order not to be called for each intermediate index.
    virtual void Execute(RenderDevice const* iRenderDevice,
                         IShaderConstantDatabase const* iShaderConstantDatabase,
                         IShaderResourceDatabase const* iShaderResourceDatabase,
                         size_t iSubLayer,
                         size_t& ioNextSubLayer) = 0;
    virtual void PostExecute() = 0;
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
}
}

#endif
