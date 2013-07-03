#ifndef RenderEngine_CompositingLayer_H
#define RenderEngine_CompositingLayer_H

#include <Core/ComPtr.h>
#include <Core/SmartPtr.h>
#include <Rendering/RenderBatchDico.h>
#include <Rendering/RenderBatchSet.h>
#include <unordered_set>
#include "CompositingInstruction.h"

namespace sg {
namespace rendering {
    class IRenderBatch;
    class IShaderConstantDatabase;
    class IShaderResourceDatabase;
    class RenderDevice;
    class ShaderConstantBuffers;
}
}

namespace sg {
namespace renderengine {
//=============================================================================
class Compositing;
//=============================================================================
class CompositingLayerDescriptor;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class CompositingLayer : public ICompositingInstruction
{
    friend class CompositingLayerDescriptor;
public:
    void Register(rendering::IRenderBatch* iInstruction);
    void Unregister(rendering::IRenderBatch* iInstruction);
    rendering::TransientRenderBatch* GetOrCreateTransientRenderBatch(
        rendering::Material const& iMaterial,
        rendering::TransientRenderBatch::Properties const& iProperties);
    rendering::RenderBatchSet* GetRenderBatchSet() { return &m_instructions; }
private:
    CompositingLayer(CompositingLayerDescriptor const* iDescriptor, Compositing* iCompositing);
    virtual ~CompositingLayer() override;
    virtual void Execute(Compositing* iCompositing) override;
private:
    safeptr<CompositingLayerDescriptor const> m_descriptor;
    rendering::RenderBatchSet m_instructions;
    rendering::RenderBatchDico m_dico;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class CompositingLayerDescriptor : public AbstractCompositingInstructionDescriptor
{
    REFLECTION_CLASS_HEADER(CompositingLayerDescriptor, AbstractCompositingInstructionDescriptor)
public:
    CompositingLayerDescriptor();
    virtual ~CompositingLayerDescriptor() override;
    virtual CompositingLayer* CreateInstance(Compositing* iCompositing) const override;
    bool DoesMatch(ArrayView<FastSymbol const> const& iSpecRequest) const;
private:
    std::vector<FastSymbol> m_spec;
};
//=============================================================================
}
}

#endif
