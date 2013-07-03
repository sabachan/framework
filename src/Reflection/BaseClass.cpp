#include "stdafx.h"

#include "BaseClass.h"

namespace sg {
namespace reflection {
//=============================================================================
char const*const sg_reflection_namespace[] = { "sg", "reflection", nullptr};
//=============================================================================
Metaclass* BaseType::sg_reflection_CreateMetaclass(MetaclassRegistrator* iRegistrator)
{
    Metaclass* mc = new Metaclass(iRegistrator, nullptr);
    return mc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
MetaclassRegistrator BaseType::s_sg_reflection_metaclassRegistrator(
        sg_reflection_namespace,
        SG_ARRAYSIZE(sg_reflection_namespace)-1,
        "BaseType",
        nullptr,
        BaseType::sg_reflection_CreateMetaclass,
        false,
        false,
        true,
        true
    );
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_REFLECTION_IMPL_DEFINE_force_registration(BaseType, BaseType)
//=============================================================================
BaseClass::BaseClass()
    : m_onCreatedCalled(false)
#if SG_ENABLE_ASSERT
    , m_creationAborted(false)
    , m_isBeingCreated(false)
    , m_isBeingModified(false)
    , m_virtualCheckPropertiesCalled(false)
#endif
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BaseClass::~BaseClass()
{
    // NB: If a class deriving from BaseClass is allowed to be instanciated
    // from the code, not a script, it should expose a dedicated constructor,
    // different from the default constructor, that calls EndCreationIFN().
    // If you need to expose a "default" constructor, you can use the tag
    // auto_initialized_t.
    // While you are here, note that it is better, for classes that inherit
    // from BaseClass, to hide the default constructor in private or protected
    // section. This forbids the usage by others than reflection code.
    SG_ASSERT(m_onCreatedCalled || m_creationAborted);
    SG_ASSERT(!m_isBeingModified);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::GetPropertyIFP(char const* iName, refptr<IPrimitiveData>* oData) const
{
    SG_ASSERT(!m_creationAborted);
    SG_ASSERT(!m_onCreatedCalled || m_isBeingModified || m_virtualCheckPropertiesCalled);
    IProperty const* property = GetMetaclass()->GetPropertyIFP(iName);
    if(nullptr == property)
        *oData = nullptr;
    else
        property->Get(this, oData);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::GetProperty(IProperty const* iProperty, refptr<IPrimitiveData>* oData) const
{
    SG_ASSERT(!m_creationAborted);
    SG_ASSERT(GetMetaclass()->GetPropertyIFP(iProperty->Name()) == iProperty);
    iProperty->Get(this, oData);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool BaseClass::SetPropertyROK(char const* iName, IPrimitiveData const* iValue)
{
    SG_ASSERT(!m_creationAborted);
    SG_ASSERT(!m_onCreatedCalled || m_isBeingModified);
    SG_ASSERT(!m_virtualCheckPropertiesCalled);
    IProperty const* property = GetMetaclass()->GetPropertyIFP(iName);
    if(nullptr == property)
        return nullptr;
    return property->SetROK(this, iValue);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool BaseClass::SetPropertyROK(IProperty const* iProperty, IPrimitiveData const* iValue)
{
    SG_ASSERT(!m_creationAborted);
    SG_ASSERT(GetMetaclass()->GetPropertyIFP(iProperty->Name()) == iProperty);
    return iProperty->SetROK(this, iValue);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::EndCreationIFN(ObjectCreationContext& iContext) const
{
    const_cast<BaseClass*>(this)->EndCreationIFN(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::EndCreationIFN(ObjectCreationContext& iContext)
{
    SG_ASSERT(!m_creationAborted);
    SG_ASSERT_MSG(!m_isBeingCreated, "There is a cyclic dependency!");
    SG_ASSERT(!m_isBeingModified);
    if(m_onCreatedCalled)
        return;
#if ENABLE_REFLECTION_PROPERTY_CHECK
    ObjectPropertyCheckContext propertyCheckContext;
    CheckProperties(propertyCheckContext);
#endif
    SG_ASSERT(!m_onCreatedCalled);
    SG_CODE_FOR_ASSERT(m_isBeingCreated = true);
    VirtualOnCreated(iContext);
    SG_CODE_FOR_ASSERT(m_isBeingCreated = false);
    SG_ASSERT_MSG(m_onCreatedCalled, "a call to reflection_parent_type::VirtualOnCreated has been forgotten");
    SG_ASSERT(m_virtualCheckPropertiesCalled);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::AbortCreation()
{
    SG_ASSERT(!m_creationAborted);
    SG_ASSERT(!m_isBeingModified);
    SG_ASSERT(!m_onCreatedCalled);
    SG_CODE_FOR_ASSERT(m_creationAborted = true);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::VirtualOnCreated(ObjectCreationContext& iContext)
{
    SG_UNUSED(iContext);
    SG_ASSERT(!m_creationAborted);
    SG_ASSERT(!m_onCreatedCalled);
    m_onCreatedCalled = true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::BeginModification()
{
    SG_ASSERT(!m_creationAborted);
    SG_ASSERT(m_onCreatedCalled);
    SG_ASSERT(!m_isBeingModified);
    SG_CODE_FOR_ASSERT(m_isBeingModified = true);
    SG_CODE_FOR_ASSERT(m_virtualCheckPropertiesCalled = false);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::EndModification(ObjectModificationContext& iContext)
{
    SG_ASSERT(!m_creationAborted);
#if ENABLE_REFLECTION_PROPERTY_CHECK
    ObjectPropertyCheckContext propertyCheckContext;
    CheckProperties(propertyCheckContext);
#endif
    SG_ASSERT(m_onCreatedCalled);
    SG_ASSERT(m_isBeingModified);
    VirtualOnModified(iContext);
    SG_ASSERT_MSG(!m_isBeingModified, "a call to reflection_parent_type::VirtualOnModified has been forgotten");
    SG_ASSERT(m_virtualCheckPropertiesCalled);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::VirtualOnModified(ObjectModificationContext& iContext)
{
    SG_UNUSED(iContext);
    SG_ASSERT(!m_creationAborted);
    SG_ASSERT(m_isBeingModified);
    SG_CODE_FOR_ASSERT(m_isBeingModified = false);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void BaseClass::CheckProperties(ObjectPropertyCheckContext& iContext) const
{
    SG_ASSERT(!m_creationAborted);
    SG_CODE_FOR_ASSERT(m_virtualCheckPropertiesCalled = false);
    SG_ASSERT(!m_virtualCheckPropertiesCalled);
    VirtualCheckProperties(iContext);
    SG_CODE_FOR_ASSERT(Metaclass const* metaclass = GetMetaclass());
    SG_ASSERT(nullptr != metaclass);
    SG_CODE_FOR_ASSERT(std::string className = metaclass->FullClassName().AsString());
    SG_ASSERT_MSG(m_virtualCheckPropertiesCalled, "a call to reflection_parent_type::VirtualCheckProperties has been forgotten.");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseClass::VirtualCheckProperties(ObjectPropertyCheckContext& iContext) const
{
    SG_UNUSED(iContext);
    SG_ASSERT(!m_virtualCheckPropertiesCalled);
    SG_CODE_FOR_ASSERT(m_virtualCheckPropertiesCalled = true);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Metaclass* BaseClass::sg_reflection_CreateMetaclass(MetaclassRegistrator* iRegistrator)
{
    Metaclass* mc = new Metaclass(iRegistrator, nullptr);
    return mc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Metaclass const* BaseClass::GetMetaclass() const
{
    Metaclass const* mc = VirtualGetMetaclass();
    SG_ASSERT(nullptr != mc);
    return mc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
MetaclassRegistrator BaseClass::s_sg_reflection_metaclassRegistrator(
        sg_reflection_namespace,
        SG_ARRAYSIZE(sg_reflection_namespace)-1,
        "BaseClass",
        &BaseType::s_sg_reflection_metaclassRegistrator,
        BaseClass::sg_reflection_CreateMetaclass,
        true,
        false,
        false,
        true
    );
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_REFLECTION_IMPL_DEFINE_force_registration(BaseClass, BaseClass)
//=============================================================================
}
}
