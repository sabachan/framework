#include "stdafx.h"

#include "ShaderDescriptors.h"

#include <Reflection/CommonTypes.h>

namespace sg {
namespace renderengine {
//=============================================================================
BaseShaderDescriptor::BaseShaderDescriptor()
: m_file()
, m_entryPoint()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BaseShaderDescriptor::~BaseShaderDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void BaseShaderDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    SG_ASSERT(!m_file.Empty() || m_entryPoint.empty());
    SG_ASSERT(!m_entryPoint.empty() || m_file.Empty());
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_ABSTRACT_CLASS_BEGIN((sg, renderengine), BaseShaderDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(file, "")
    REFLECTION_m_PROPERTY_DOC(entryPoint, "")
    REFLECTION_m_PROPERTY_DOC(defines, "")
REFLECTION_CLASS_END
//=============================================================================
PixelShaderDescriptor::PixelShaderDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PixelShaderDescriptor::~PixelShaderDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::PixelShaderProxy PixelShaderDescriptor::GetProxy() const
{
    if(!m_proxy.IsValid() && IsValid())
    {
        refptr<rendering::PixelShaderDescriptor> desc = new rendering::PixelShaderDescriptor(m_file, m_entryPoint, m_defines.View());
        m_proxy = desc->GetProxy();
    }
    return m_proxy;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), PixelShaderDescriptor)
REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
VertexShaderDescriptor::VertexShaderDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
VertexShaderDescriptor::~VertexShaderDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::VertexShaderProxy VertexShaderDescriptor::GetProxy() const
{
    if(!m_proxy.IsValid() && IsValid())
    {
        refptr<rendering::VertexShaderDescriptor> desc = new rendering::VertexShaderDescriptor(m_file, m_entryPoint);
        m_proxy = desc->GetProxy();
    }
    return m_proxy;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), VertexShaderDescriptor)
REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
}
}
