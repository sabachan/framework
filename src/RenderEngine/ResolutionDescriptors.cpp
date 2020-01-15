#include "stdafx.h"

#include "ResolutionDescriptors.h"

#include "Compositing.h"
#include "SurfaceDescriptors.h"
#include <Reflection/CommonTypes.h>

namespace sg {
namespace renderengine {
//=============================================================================
ResolutionInstance::ResolutionInstance(AbstractResolutionDescriptor const* iDescriptor, rendering::ResolutionServer const*const (&iParent)[MAX_PARENT_COUNT])
: m_descriptor(iDescriptor)
{
    SG_CODE_FOR_ASSERT(bool endOfParents = false;)
    for(size_t i = 0; i < MAX_PARENT_COUNT; ++i)
    {
        SG_ASSERT(!endOfParents || nullptr == iParent[i]);
        SG_CODE_FOR_ASSERT(if(nullptr == iParent[i]) endOfParents = true;)
        m_parents[i] = iParent[i];
    }
    for(size_t i = 0; i < MAX_PARENT_COUNT; ++i)
    {
        bool alreadyRegistered = false;
        for(size_t j = 0; j < i; ++j)
        {
            if(m_parents[i] == m_parents[j])
                alreadyRegistered = true;
        }
        if(!alreadyRegistered)
            m_parents[i]->RegisterObserver(this);
    }
    Update();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ResolutionInstance::~ResolutionInstance()
{
    for(size_t i = 0; i < MAX_PARENT_COUNT; ++i)
    {
        bool alreadyUnregistered = false;
        for(size_t j = 0; j < i; ++j)
        {
            if(m_parents[i] == m_parents[j])
            {
                alreadyUnregistered = true;
                break;
            }
        }
        if(!alreadyUnregistered)
            m_parents[i]->UnregisterObserver(this);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ResolutionInstance::VirtualOnNotified(ResolutionServer const* iResolutionServer)
{
    SG_UNUSED(iResolutionServer);
#if SG_ENABLE_ASSERT
    bool found = false;
    for(size_t i = 0; i < MAX_PARENT_COUNT; ++i)
    {
        if(m_parents[i] == iResolutionServer)
        {
            found = true;
            break;
        }
    }
    SG_ASSERT(found);
#endif
    Update();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ResolutionInstance::Update()
{
    uint2 parentResolutions[MAX_PARENT_COUNT] = { uint2(0), };
    for(size_t i = 0; i < MAX_PARENT_COUNT; ++i)
    {
        if(nullptr != m_parents[i])
            parentResolutions[i] = m_parents[i]->Get();
    }
    uint2 const resolution = m_descriptor->ComputeResolution(parentResolutions);
    Set(resolution);
}
//=============================================================================
AbstractResolutionDescriptor::AbstractResolutionDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
AbstractResolutionDescriptor::~AbstractResolutionDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_ABSTRACT_CLASS_BEGIN((sg, renderengine), AbstractResolutionDescriptor)
REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
ResolutionFromInputSurfaceDescriptor::ResolutionFromInputSurfaceDescriptor()
: m_surface(nullptr)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ResolutionFromInputSurfaceDescriptor::~ResolutionFromInputSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::ResolutionServer const* ResolutionFromInputSurfaceDescriptor::CreateInstance(Compositing* iCompositing) const
{
    rendering::IShaderResource const* shaderResource = m_surface->GetAsShaderResource(iCompositing);
    return shaderResource->ShaderResourceResolution();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
uint2 ResolutionFromInputSurfaceDescriptor::ComputeResolution(uint2 const (&iParentResolutions)[ResolutionInstance::MAX_PARENT_COUNT]) const
{
    SG_UNUSED(iParentResolutions);
    SG_ASSERT_NOT_REACHED();
    return uint2(0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), ResolutionFromInputSurfaceDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(surface, "")
REFLECTION_CLASS_END
//=============================================================================
ResolutionFromOutputSurfaceDescriptor::ResolutionFromOutputSurfaceDescriptor()
: m_surface(nullptr)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ResolutionFromOutputSurfaceDescriptor::~ResolutionFromOutputSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::ResolutionServer const* ResolutionFromOutputSurfaceDescriptor::CreateInstance(Compositing* iCompositing) const
{
    rendering::IRenderTarget const* renderTarget = m_surface->GetAsRenderTarget(iCompositing);
    return renderTarget->RenderTargetResolution();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
uint2 ResolutionFromOutputSurfaceDescriptor::ComputeResolution(uint2 const (&iParentResolutions)[ResolutionInstance::MAX_PARENT_COUNT]) const
{
    SG_UNUSED(iParentResolutions);
    SG_ASSERT_NOT_REACHED();
    return uint2(0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), ResolutionFromOutputSurfaceDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(surface, "")
REFLECTION_CLASS_END
//=============================================================================
ResolutionFromTextureSurfaceDescriptor::ResolutionFromTextureSurfaceDescriptor()
: m_surface(nullptr)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ResolutionFromTextureSurfaceDescriptor::~ResolutionFromTextureSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::ResolutionServer const* ResolutionFromTextureSurfaceDescriptor::CreateInstance(Compositing* iCompositing) const
{
    rendering::IShaderResource const* renderTarget = m_surface->GetAsShaderResource(iCompositing);
    return renderTarget->ShaderResourceResolution();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
uint2 ResolutionFromTextureSurfaceDescriptor::ComputeResolution(uint2 const (&iParentResolutions)[ResolutionInstance::MAX_PARENT_COUNT]) const
{
    SG_UNUSED(iParentResolutions);
    SG_ASSERT_NOT_REACHED();
    return uint2(0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), ResolutionFromTextureSurfaceDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(surface, "")
REFLECTION_CLASS_END
//=============================================================================
ResolutionDescriptor::ResolutionDescriptor()
: m_parentx(nullptr)
, m_parenty(nullptr)
, m_scale(0)
, m_roundingMode(RoundingMode::Floor)
, m_additiveTerm(0)
SG_CODE_FOR_ASSERT(SG_COMMA m_isInCreateInstance(false))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ResolutionDescriptor::~ResolutionDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::ResolutionServer const* ResolutionDescriptor::CreateInstance(Compositing* iCompositing) const
{
    SG_ASSERT(!m_isInCreateInstance);
    SG_CODE_FOR_ASSERT(m_isInCreateInstance = true;)
    rendering::ResolutionServer const* parents[ResolutionInstance::MAX_PARENT_COUNT] = { nullptr, };
    parents[0] = iCompositing->GetResolutionServer(m_parentx.get());
    parents[1] = iCompositing->GetResolutionServer(m_parenty.get());
    ResolutionInstance* instance = new ResolutionInstance(this, parents);
    iCompositing->RegisterResolutionInstanceForHandling(instance);
    SG_CODE_FOR_ASSERT(m_isInCreateInstance = false;)
    return instance;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
uint2 ResolutionDescriptor::ComputeResolution(uint2 const (&iParentResolutions)[ResolutionInstance::MAX_PARENT_COUNT]) const
{
    uint2 wh = uint2(iParentResolutions[0].x(), iParentResolutions[1].y());
    float2 scaledwh = float2(wh) * m_scale;
    switch(m_roundingMode)
    {
    case RoundingMode::Floor:
        wh = uint2(floori(scaledwh));
        break;
    case RoundingMode::Round:
        wh = uint2(roundi(scaledwh));
        break;
    case RoundingMode::Ceil:
        wh = uint2(ceili(scaledwh));
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    wh += m_additiveTerm;

    return wh;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void ResolutionDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    REFLECTION_CHECK_PROPERTY(scale, m_scale.x() >= 0);
    REFLECTION_CHECK_PROPERTY(scale, m_scale.y() >= 0);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_QUALIFIED_ENUM_BEGIN(ResolutionDescriptor::RoundingMode, ResolutionDescriptor_RoundingMode)
REFLECTION_ENUM_DOC("")
    REFLECTION_ENUM_VALUE_DOC(Floor, "")
    REFLECTION_ENUM_VALUE_DOC(Ceil, "")
    REFLECTION_ENUM_VALUE_DOC(Round, "")
REFLECTION_ENUM_END
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), ResolutionDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(parentx, "")
    REFLECTION_m_PROPERTY_DOC(parenty, "")
    REFLECTION_m_PROPERTY_DOC(scale, "")
    REFLECTION_m_PROPERTY_DOC(roundingMode, "")
    REFLECTION_m_PROPERTY_DOC(additiveTerm, "")
REFLECTION_CLASS_END
//=============================================================================
}
}
