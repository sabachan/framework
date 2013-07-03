#include "stdafx.h"

#include "CompositingLayer.h"

#include <Core/Log.h>
#include <Reflection/CommonTypes.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/RenderBatch.h>
#include <Rendering/RenderStateUtils.h>
#include <Rendering/ShaderConstantBuffers.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <Rendering/ShaderResourceBuffer.h>
#include <Rendering/Surface.h>
#include <Rendering/VertexTypes.h>
#include <algorithm>
#include <d3d11.h>
#include <d3d11shader.h>
#include "Compositing.h"
#include "DatabaseDescriptors.h"
#include "RenderBatch.h"
#include "ShaderDescriptors.h"
#include "SurfaceDescriptors.h"
#include <algorithm>

namespace sg {
namespace renderengine {
//=============================================================================
CompositingLayer::CompositingLayer(CompositingLayerDescriptor const* iDescriptor, Compositing* iCompositing)
: m_descriptor(iDescriptor)
, m_instructions()
, m_dico(&m_instructions)
{
    SG_UNUSED(iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CompositingLayer::~CompositingLayer()
{
    m_dico.Clear(&m_instructions);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CompositingLayer::Register(rendering::IRenderBatch* iInstruction)
{
    m_instructions.Insert(iInstruction);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CompositingLayer::Unregister(rendering::IRenderBatch* iInstruction)
{
    m_instructions.Remove(iInstruction);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::TransientRenderBatch* CompositingLayer::GetOrCreateTransientRenderBatch(
    rendering::Material const& iMaterial,
    rendering::TransientRenderBatch::Properties const& iProperties)
{
    return m_dico.GetOrCreateTransientRenderBatch(&m_instructions, iMaterial, iProperties);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CompositingLayer::Execute(Compositing* iCompositing)
{
    rendering::RenderDevice const* renderDevice = iCompositing->RenderDevice();
    rendering::IShaderConstantDatabase const* shaderConstantDatabase = iCompositing->GetCurrentShaderConstantDatabase();
    rendering::IShaderResourceDatabase const* shaderResourceDatabase = iCompositing->GetCurrentShaderResourceDatabase();
    m_instructions.Execute(renderDevice, shaderConstantDatabase, shaderResourceDatabase);
}
//=============================================================================
CompositingLayerDescriptor::CompositingLayerDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CompositingLayerDescriptor::~CompositingLayerDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CompositingLayer* CompositingLayerDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new CompositingLayer(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CompositingLayerDescriptor::DoesMatch(ArrayView<FastSymbol const> const& iSpecRequest) const
{
    auto begin = m_spec.begin();
    auto end = m_spec.end();
    for(auto it : iSpecRequest)
    {
        if(end == std::find(begin, end, it))
            return false;
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), CompositingLayerDescriptor)
REFLECTION_CLASS_DOC("")
REFLECTION_m_PROPERTY_DOC(spec,
"Defines the specifications of the layer.\n"
"It is a list of identifiers that can be matched to a spec request with same form. "
"The only layer containing all identifiers of the request is considered matching "
"the request.")
REFLECTION_CLASS_END
//=============================================================================
}
}

