#include "stdafx.h"

#include "MaterialDescriptor.h"

#include "Compositing.h"
#include "SamplerDescriptor.h"
#include "ShaderDescriptors.h"
#include "SurfaceDescriptors.h"
#include <Rendering/RenderDevice.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <Rendering/VertexTypes.h>
#include <d3d11.h>

namespace sg {
namespace renderengine {
//=============================================================================
rendering::Material const* MaterialDescriptorCache::GetMaterial(
    ArrayView<D3D11_INPUT_ELEMENT_DESC const> iInputLayout,
    rendering::VertexShaderProxy iVertexShader,
    rendering::PixelShaderProxy iPixelShader,
    rendering::RenderStateName iBlendState,
    ConstantsView iConstants,
    ResourcesView iResources,
    SamplersView iSamplers,
    int iPriority)
{
    rendering::VertexShaderProxy vs = iVertexShader;
    rendering::ShaderInputLayoutProxy layout(iInputLayout, vs);

    LightKey key;
    key.inputLayout = layout;
    key.resources = iResources;
    auto r = m_materials.emplace(key, nullptr);
    if(r.second)
    {
        rendering::PixelShaderProxy ps = iPixelShader;
        rendering::Material const* material = new rendering::Material(layout, vs, ps, iBlendState, iConstants, iResources, iSamplers, iPriority);
        r.first->second = material;
        return material;
    }
    else
    {
        rendering::Material const* material = r.first->second.get();
        SG_ASSERT(nullptr != material);
        return material;
    }
}
//=============================================================================
MaterialDescriptor::MaterialDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
MaterialDescriptor::~MaterialDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::Material const* MaterialDescriptor::GetMaterial(
    ArrayView<D3D11_INPUT_ELEMENT_DESC const> iInputLayout,
    ConstantsView iConstants,
    ResourcesView iResources,
    SamplersView iSamplers)
{
    SG_ASSERT_MSG(iConstants.Empty(), "not supported yet");
    SG_ASSERT_MSG(iSamplers.Empty(), "not supported yet");
    if(nullptr == m_cache) { CreateCache(); }
    return m_cache->GetMaterial(
        iInputLayout,
        m_vertexShader->GetProxy(),
        m_pixelShader->GetProxy(),
        m_blendState,
        iConstants,
        iResources,
        iSamplers,
        int(m_priority));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void MaterialDescriptor::CreateCache()
{
    m_cache.reset(new MaterialDescriptorCache());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void MaterialDescriptor::OnCreatedOrModified()
{
    if(!m_blendState.IsValid())
        m_blendState = "Premultiplied Alpha Blending";
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void MaterialDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), MaterialDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(vertexShader, "")
    REFLECTION_m_PROPERTY_DOC(pixelShader, "")
    REFLECTION_m_PROPERTY_DOC(blendState, "")
    REFLECTION_m_PROPERTY_DOC(priority, "")
REFLECTION_CLASS_END
//=============================================================================
}
}

