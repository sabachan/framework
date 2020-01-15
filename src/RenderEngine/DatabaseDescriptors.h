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
class GenericConstantDatabaseInstance : public RefAndSafeCountableWithVirtualDestructor
{
public:
    GenericConstantDatabaseInstance(rendering::IShaderConstantDatabase const* iDatabase);
    ~GenericConstantDatabaseInstance() override;
    SG_FORCE_INLINE rendering::IShaderConstantDatabase const* GetDatabase() const { return m_database.get(); }
private:
    refptr<rendering::IShaderConstantDatabase const> m_database;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class AbstractConstantDatabaseDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(AbstractConstantDatabaseDescriptor, reflection::BaseClass)
public:
    typedef GenericConstantDatabaseInstance instance_type;
    AbstractConstantDatabaseDescriptor();
    virtual ~AbstractConstantDatabaseDescriptor() override;
protected:
    bool IsEmpty() const { return m_constants.empty(); }
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
    typedef GenericConstantDatabaseInstance instance_type;
    InputConstantDatabaseDescriptor();
    virtual ~InputConstantDatabaseDescriptor() override;
    GenericConstantDatabaseInstance* CreateInstance(rendering::IShaderConstantDatabase const* iInputDatabase) const;
private:
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
};
//=============================================================================
class ConstantDatabaseInstance : public GenericConstantDatabaseInstance
{
public:
    ConstantDatabaseInstance(rendering::ShaderConstantDatabase* iWritableDatabase, rendering::IShaderConstantDatabase const* iDatabase);
    ~ConstantDatabaseInstance() override;
    SG_FORCE_INLINE rendering::ShaderConstantDatabase* GetWritableDatabase() const { return m_writableDatabase.get(); }
private:
    refptr<rendering::ShaderConstantDatabase> m_writableDatabase;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ConstantDatabaseDescriptor : public AbstractConstantDatabaseDescriptor
{
    REFLECTION_CLASS_HEADER(ConstantDatabaseDescriptor, AbstractConstantDatabaseDescriptor)
public:
    typedef ConstantDatabaseInstance instance_type;
    ConstantDatabaseDescriptor();
    virtual ~ConstantDatabaseDescriptor() override;
    ConstantDatabaseInstance* CreateInstance(Compositing* iCompositing) const;
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
    refptr<AbstractShaderResourceDatabaseDescriptor> m_parent;
};
//=============================================================================
}
}
#endif
