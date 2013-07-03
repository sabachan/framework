#ifndef RenderEngine_DatabaseDescriptors_H
#define RenderEngine_DatabaseDescriptors_H

#include <Core/SmartPtr.h>
#include <Reflection/BaseClass.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <Rendering/ShaderResourceDatabase.h>

namespace sg {
namespace rendering {
    class IRenderTarget;
    class RenderDevice;
    class ShaderConstantDatabase;
    class ShaderResourceDatabase;
}
}

namespace sg {
namespace renderengine {
//=============================================================================
class AbstractSurfaceDescriptor;
class Compositing;
class InputSurfaceDescriptor;
class SamplerDescriptor;
//=============================================================================
class AbstractConstantDatabaseDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(AbstractConstantDatabaseDescriptor, reflection::BaseClass)
public:
    AbstractConstantDatabaseDescriptor();
    virtual ~AbstractConstantDatabaseDescriptor() override;
protected:
    void PopulateDatabase(rendering::ShaderConstantDatabase* ioDatabase) const;
protected:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    std::vector<std::pair<rendering::ShaderConstantName, refptr<reflection::IPrimitiveData const> > > m_constants;
};
//=============================================================================
class InputConstantDatabaseDescriptor : public AbstractConstantDatabaseDescriptor
{
    REFLECTION_CLASS_HEADER(InputConstantDatabaseDescriptor, AbstractConstantDatabaseDescriptor)
public:
    InputConstantDatabaseDescriptor();
    virtual ~InputConstantDatabaseDescriptor() override;
    rendering::IShaderConstantDatabase* CreateDatabase(rendering::IShaderConstantDatabase const* iInputDatabase) const;
private:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
};
//=============================================================================
class ConstantDatabaseDescriptor : public AbstractConstantDatabaseDescriptor
{
    REFLECTION_CLASS_HEADER(ConstantDatabaseDescriptor, AbstractConstantDatabaseDescriptor)
public:
    ConstantDatabaseDescriptor();
    virtual ~ConstantDatabaseDescriptor() override;
    rendering::IShaderConstantDatabase* CreateDatabase(Compositing* iCompositing) const;
private:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    refptr<AbstractConstantDatabaseDescriptor> m_parent;
};
//=============================================================================
class AbstractShaderResourceDatabaseDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(AbstractShaderResourceDatabaseDescriptor, reflection::BaseClass)
public:
    AbstractShaderResourceDatabaseDescriptor();
    virtual ~AbstractShaderResourceDatabaseDescriptor() override;
protected:
    void PopulateDatabase(Compositing* iCompositing, rendering::ShaderResourceDatabase* ioDatabase) const;
protected:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    std::vector<std::pair<rendering::ShaderResourceName, refptr<AbstractSurfaceDescriptor> > > m_resources;
    std::vector<std::pair<rendering::ShaderResourceName, refptr<SamplerDescriptor> > > m_samplers;
};
//=============================================================================
class InputShaderResourceDatabaseDescriptor : public AbstractShaderResourceDatabaseDescriptor
{
    REFLECTION_CLASS_HEADER(InputShaderResourceDatabaseDescriptor, AbstractShaderResourceDatabaseDescriptor)
public:
    InputShaderResourceDatabaseDescriptor();
    virtual ~InputShaderResourceDatabaseDescriptor() override;
    rendering::IShaderResourceDatabase* CreateDatabase(Compositing* iCompositing, rendering::IShaderResourceDatabase const* iInputDatabase) const;
private:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
};
//=============================================================================
class ShaderResourceDatabaseDescriptor : public AbstractShaderResourceDatabaseDescriptor
{
    REFLECTION_CLASS_HEADER(ShaderResourceDatabaseDescriptor, AbstractShaderResourceDatabaseDescriptor)
public:
    ShaderResourceDatabaseDescriptor();
    virtual ~ShaderResourceDatabaseDescriptor() override;
    rendering::IShaderResourceDatabase* CreateDatabase(Compositing* iCompositing) const;
private:
    refptr<ShaderResourceDatabaseDescriptor> m_parent;
};
//=============================================================================
}
}
#endif
