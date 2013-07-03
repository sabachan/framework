#ifndef RenderEngine_ShaderDescriptors_H
#define RenderEngine_ShaderDescriptors_H

#include <Core/ComPtr.h>
#include <Core/FilePath.h>
#include <Core/SmartPtr.h>
#include <Reflection/BaseClass.h>
#include <Rendering/Shader.h>

namespace sg {
namespace renderengine {
//=============================================================================
class BaseShaderDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(BaseShaderDescriptor, reflection::BaseClass)
public:
    bool IsValid() const { return !m_file.Empty(); }
protected:
    BaseShaderDescriptor();
    virtual ~BaseShaderDescriptor() override;
protected:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
protected:
    FilePath m_file;
    std::string m_entryPoint;
    // TODO: defines
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class PixelShaderDescriptor : public BaseShaderDescriptor
{
    REFLECTION_CLASS_HEADER(PixelShaderDescriptor, BaseShaderDescriptor)
public:
    PixelShaderDescriptor();
    virtual ~PixelShaderDescriptor() override;
    rendering::PixelShaderProxy GetProxy() const;
private:
    mutable rendering::PixelShaderProxy m_proxy;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class VertexShaderDescriptor : public BaseShaderDescriptor
{
    REFLECTION_CLASS_HEADER(VertexShaderDescriptor, BaseShaderDescriptor)
public:
    VertexShaderDescriptor();
    virtual ~VertexShaderDescriptor() override;
    rendering::VertexShaderProxy GetProxy() const;
private:
    mutable rendering::VertexShaderProxy m_proxy;
};
//=============================================================================
}
}

#endif
