#include "stdafx.h"

#include "SamplerDescriptor.h"

#include <Reflection/CommonTypes.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/WTF/IncludeD3D11.h>

namespace sg {
namespace renderengine {
//=============================================================================
SamplerDescriptor::SamplerDescriptor()
: m_filter(Filter::Point)
, m_adressMode(AdressMode::Wrap)
, m_maxAnisotropy(0)
, m_borderColor(0)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SamplerDescriptor::~SamplerDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
    D3D11_FILTER const filterConversion[] = {
        D3D11_FILTER_MIN_MAG_MIP_POINT,
        D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        D3D11_FILTER_ANISOTROPIC,
    };
    D3D11_TEXTURE_ADDRESS_MODE const adressModeConversion[] = { 
        D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_MIRROR,
        D3D11_TEXTURE_ADDRESS_CLAMP,
        D3D11_TEXTURE_ADDRESS_BORDER,
        D3D11_TEXTURE_ADDRESS_MIRROR_ONCE,
    };
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SamplerDescriptor::CreateSamplerState(rendering::RenderDevice const* iRenderDevice, comptr<ID3D11SamplerState>& oSampler) const
{
    SG_ASSERT(nullptr != iRenderDevice);
    ID3D11Device* device = iRenderDevice->D3DDevice();
    SG_ASSERT(nullptr != device);

    D3D11_SAMPLER_DESC samplerDesc;
    SG_ASSERT(static_cast<unsigned>(m_filter) < SG_ARRAYSIZE(filterConversion));
    samplerDesc.Filter = filterConversion[static_cast<unsigned>(m_filter)];
    SG_ASSERT(static_cast<unsigned>(m_adressMode) < SG_ARRAYSIZE(adressModeConversion));
    samplerDesc.AddressU = adressModeConversion[static_cast<unsigned>(m_adressMode)];
    samplerDesc.AddressV = samplerDesc.AddressU;
    samplerDesc.AddressW = samplerDesc.AddressU;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = m_maxAnisotropy;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = m_borderColor[0];
    samplerDesc.BorderColor[1] = m_borderColor[1];
    samplerDesc.BorderColor[2] = m_borderColor[2];
    samplerDesc.BorderColor[3] = m_borderColor[3];
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT hr = device->CreateSamplerState(&samplerDesc, oSampler.GetPointerForInitialisation());

#if SG_ENABLE_ASSERT
    rendering::SetDebugName(oSampler.get(), "SamplerDescriptor");
#endif

    SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void SamplerDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_QUALIFIED_ENUM_BEGIN(SamplerDescriptor::Filter, SamplerDescriptor_Filter)
REFLECTION_ENUM_DOC("")
    REFLECTION_ENUM_VALUE_DOC(Point, "")
    REFLECTION_ENUM_VALUE_DOC(Linear, "")
    REFLECTION_ENUM_VALUE_DOC(Anisotropic, "")
REFLECTION_ENUM_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_QUALIFIED_ENUM_BEGIN(SamplerDescriptor::AdressMode, SamplerDescriptor_AdressMode)
REFLECTION_ENUM_DOC("")
    REFLECTION_ENUM_VALUE_DOC(Wrap, "")
    REFLECTION_ENUM_VALUE_DOC(Mirror, "")
    REFLECTION_ENUM_VALUE_DOC(Clamp, "")
    REFLECTION_ENUM_VALUE_DOC(Border, "")
    REFLECTION_ENUM_VALUE_DOC(MirrorOnce, "")
REFLECTION_ENUM_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), SamplerDescriptor)
    REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(filter, "")
    REFLECTION_m_PROPERTY_DOC(adressMode, "")
    REFLECTION_m_PROPERTY_DOC(maxAnisotropy, "")
    REFLECTION_m_PROPERTY_DOC(borderColor, "")
REFLECTION_CLASS_END
//=============================================================================
}
}
