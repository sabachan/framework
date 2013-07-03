#ifndef Reflection_BaseClass_H
#define Reflection_BaseClass_H

#include "Metaclass.h"

#include "PrimitiveData.h"
#include "Property.h"
#include <Core/SmartPtr.h>
#include <Core/Utils.h>

#define ENABLE_REFLECTION_PROPERTY_CHECK SG_ENABLE_TOOLS

namespace sg {
namespace reflection {
//=============================================================================
class Metaclass;
struct MetaclassRegistrator;
//=============================================================================
class ObjectCreationContext
{
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ObjectModificationContext
{
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
class ObjectPropertyCheckContext
{
};
#endif
//=============================================================================
struct BaseType
{
public:
    static Metaclass const* StaticGetMetaclass() { return s_sg_reflection_metaclassRegistrator.metaclass; }
private:
    static Metaclass* sg_reflection_CreateMetaclass(MetaclassRegistrator* iRegistrator);
public:
    static sg::reflection::MetaclassRegistrator s_sg_reflection_metaclassRegistrator;
};
//=============================================================================
// This class is the base class of any class that can be instanciated using
// reflection.
// Note that it is better, for classes that inherit from BaseClass, to hide the
// default constructor in private or protected section. This forbids the usage
// by others than reflection code.
// If you want a subclass to be instanciable by generic code also, it should
// expose a dedicated constructor, different from the default constructor (that
// should be inaccessible), that calls EndCreationIFN(). If you need to expose
// a "default" constructor, you can use the tag auto_initialized_t.
class BaseClass : public BaseType, public RefAndSafeCountable
{
public:
    BaseClass();
    virtual ~BaseClass();

    Metaclass const* GetMetaclass() const;
    void GetPropertyIFP(char const* iName, refptr<IPrimitiveData>* oData) const;
    void GetProperty(IProperty const* iProperty, refptr<IPrimitiveData>* oData) const;
    void BeginModification();
    bool SetPropertyROK(char const* iName, IPrimitiveData const* iValue);
    bool SetPropertyROK(IProperty const* iProperty, IPrimitiveData const* iValue);
    void EndModification(ObjectModificationContext& iContext);
    // An object may call EndCreationIFN on another object in its own
    // VirtualOnCreated(). In order to be able to declare this dependency on
    // const object, a const method is also exposed, that does a const_cast.
    void EndCreationIFN(ObjectCreationContext& iContext) const;
    void EndCreationIFN(ObjectCreationContext& iContext);
    void AbortCreation();
#if ENABLE_REFLECTION_PROPERTY_CHECK
    void CheckProperties(ObjectPropertyCheckContext& iContext) const;
#endif
    static Metaclass const* StaticGetMetaclass() { return s_sg_reflection_metaclassRegistrator.metaclass; }
protected:
    virtual void VirtualOnCreated(ObjectCreationContext& iContext);
    virtual void VirtualOnModified(ObjectModificationContext& iContext);
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(ObjectPropertyCheckContext& iContext) const;
#endif
    virtual Metaclass const* VirtualGetMetaclass() const { return s_sg_reflection_metaclassRegistrator.metaclass; }

private:
    static Metaclass* sg_reflection_CreateMetaclass(MetaclassRegistrator* iRegistrator);
protected:
    static sg::reflection::MetaclassRegistrator s_sg_reflection_metaclassRegistrator;
private:
    bool m_onCreatedCalled : 1;
#if SG_ENABLE_ASSERT
    bool m_creationAborted : 1;
    bool m_isBeingCreated : 1;
    bool m_isBeingModified : 1;
    mutable bool m_virtualCheckPropertiesCalled : 1;
#endif
};
//=============================================================================
template <typename T> struct MetaclassGetter { static Metaclass const* GetMetaclass() { return T::StaticGetMetaclass(); } };
//=============================================================================
}
}

#define Reflection_BaseClass_Include_MacroImpl
#include "BaseClass_MacroImpl.h"
#undef Reflection_BaseClass_Include_MacroImpl

#define REFLECTION_DECLARE_METACLASS(NS, NAME) SG_REFLECTION_IMPL_FORCE_METACLASS_REGISTRATION(NS, NAME)

// For externally defined pod types - These macros must be used in namespace sg::reflection - TYPE must be namespaced
#define REFLECTION_TYPE_WRAPPER_HEADER(NS, TYPE, NAME) SG_REFLECTION_IMPL_TYPE_WRAPPER_HEADER(NS, TYPE, NAME)
#define REFLECTION_TYPE_WRAPPER_BEGIN(NAME)            SG_REFLECTION_IMPL_TYPE_WRAPPER_BEGIN(NAME)
#define REFLECTION_TYPE_WRAPPER_DOC(DOC)               SG_REFLECTION_IMPL_TYPE_WRAPPER_DOC(DOC)
#define REFLECTION_TYPE_WRAPPER_END                    SG_REFLECTION_IMPL_TYPE_WRAPPER_END

// For enums
#define REFLECTION_ENUM_HEADER(TYPE)                        SG_REFLECTION_IMPL_ENUM_HEADER(TYPE)
#define REFLECTION_QUALIFIED_ENUM_BEGIN(TYPE, IDENTIFIER)   SG_REFLECTION_IMPL_ENUM_BEGIN(TYPE, IDENTIFIER)
#define REFLECTION_ENUM_BEGIN(TYPE)                         SG_REFLECTION_IMPL_ENUM_BEGIN(TYPE, TYPE)
#define REFLECTION_ENUM_DOC(DOC)                            SG_REFLECTION_IMPL_ENUM_DOC(DOC)
#define REFLECTION_ENUM_NAMED_VALUE_DOC(VALUE, NAME, DOC)   SG_REFLECTION_IMPL_REFLECTION_ENUM_NAMED_VALUE_DOC(VALUE, NAME, DOC)
#define REFLECTION_ENUM_VALUE_DOC(NAME, DOC)                REFLECTION_ENUM_NAMED_VALUE_DOC(NAME, NAME, DOC)
#define REFLECTION_ENUM_VALUE(NAME)                         REFLECTION_ENUM_NAMED_VALUE_DOC(NAME, NAME, "")
#define REFLECTION_ENUM_END                                 SG_REFLECTION_IMPL_ENUM_END

// For C-style enums
#define REFLECTION_C_ENUM_HEADER(TYPE)                      SG_REFLECTION_IMPL_ENUM_HEADER(TYPE)
#define REFLECTION_QUALIFIED_C_ENUM_BEGIN(TYPE, IDENTIFIER) SG_REFLECTION_IMPL_ENUM_BEGIN(TYPE, IDENTIFIER)
#define REFLECTION_C_ENUM_BEGIN(TYPE)                       SG_REFLECTION_IMPL_ENUM_BEGIN(TYPE, TYPE)
#define REFLECTION_C_ENUM_DOC(DOC)                          SG_REFLECTION_IMPL_ENUM_DOC(DOC)
#define REFLECTION_C_ENUM_NAMED_VALUE_DOC(VALUE, NAME, DOC) SG_REFLECTION_IMPL_REFLECTION_C_ENUM_NAMED_VALUE_DOC(VALUE, NAME, DOC)
#define REFLECTION_C_ENUM_VALUE_DOC(NAME, DOC)              REFLECTION_C_ENUM_NAMED_VALUE_DOC(NAME, NAME, DOC)
#define REFLECTION_C_ENUM_VALUE(NAME)                       REFLECTION_C_ENUM_NAMED_VALUE_DOC(NAME, NAME, "")
#define REFLECTION_C_ENUM_END                               SG_REFLECTION_IMPL_ENUM_END

// For structs (must derive from BaseType)
#define REFLECTION_TYPE_HEADER(TYPE, PARENT) SG_REFLECTION_IMPL_TYPE_HEADER(TYPE, PARENT)
#define REFLECTION_TYPE_BEGIN(NS, TYPE)      SG_REFLECTION_IMPL_TYPE_BEGIN(NS, TYPE)
#define REFLECTION_TYPE_DOC(DOC)             SG_REFLECTION_IMPL_TYPE_DOC(DOC)
#define REFLECTION_TYPE_END                  SG_REFLECTION_IMPL_TYPE_END
#define REFLECTION_TYPE_BEGIN_WITH_CONSTRUCTS(NS, TYPE, ENABLE_LIST, ENABLE_STRUCT)  SG_REFLECTION_IMPL_TYPE_BEGIN_WITH_CONSTRUCTS(NS, TYPE, ENABLE_LIST, ENABLE_STRUCT)

// For polymorpic classes (must derive from BaseClasses)
#define REFLECTION_CLASS_HEADER(CLASS, PARENT)                    SG_REFLECTION_IMPL_CLASS_HEADER(CLASS, PARENT)
#define REFLECTION_CLASS_BEGIN(NS, CLASS)                         SG_REFLECTION_IMPL_CLASS_BEGIN(NS, CLASS)
#define REFLECTION_ABSTRACT_CLASS_BEGIN(NS, CLASS)                SG_REFLECTION_IMPL_ABSTRACT_CLASS_BEGIN(NS, CLASS)
#define REFLECTION_TEMPLATE_CLASS_BEGIN(NS, CLASS, NAME)          SG_REFLECTION_IMPL_TEMPLATE_CLASS_BEGIN(NS, CLASS, NAME)
#define REFLECTION_TEMPLATE_ABSTRACT_CLASS_BEGIN(NS, CLASS, NAME) SG_REFLECTION_IMPL_TEMPLATE_ABSTRACT_CLASS_BEGIN(NS, CLASS, NAME)
#define REFLECTION_CLASS_DOC(DOC)                                 SG_REFLECTION_IMPL_CLASS_DOC(DOC)
#define REFLECTION_CLASS_END                                      SG_REFLECTION_IMPL_CLASS_END

#define REFLECTION_NAMED_PROPERTY_DOC(MEMBER, NAME, DOC) SG_REFLECTION_IMPL_REFLECTION_NAMED_PROPERTY_DOC(MEMBER, NAME, DOC)
#define REFLECTION_NAMED_PROPERTY(MEMBER, NAME) REFLECTION_NAMED_PROPERTY_DOC(MEMBER, NAME, "")
#define REFLECTION_m_PROPERTY_DOC(NAME, DOC) REFLECTION_NAMED_PROPERTY_DOC(m_##NAME, NAME, DOC)
#define REFLECTION_m_PROPERTY(NAME) REFLECTION_NAMED_PROPERTY_DOC(m_##NAME, NAME, "")
#define REFLECTION_PROPERTY_DOC(NAME, DOC) REFLECTION_NAMED_PROPERTY_DOC(NAME, NAME, DOC)
#define REFLECTION_PROPERTY(NAME) REFLECTION_NAMED_PROPERTY_DOC(NAME, NAME, "")

#if ENABLE_REFLECTION_PROPERTY_CHECK
#define REFLECTION_CHECK_PROPERTY(NAME, TEST)                   SG_REFLECTION_IMPL_CHECK_PROPERTY(NAME, TEST)
#define REFLECTION_CHECK_PROPERTY_MSG(NAME, TEST, MSG)          SG_REFLECTION_IMPL_CHECK_PROPERTY_MSG(NAME, TEST, MSG)
#define REFLECTION_CHECK_PROPERTY_NotNull(NAME)                 SG_REFLECTION_IMPL_CHECK_PROPERTY_NotNull(NAME)
#define REFLECTION_CHECK_PROPERTY_Greater(NAME, VAL)            SG_REFLECTION_IMPL_CHECK_PROPERTY_Greater(NAME, VAL)
#define REFLECTION_CHECK_PROPERTY_GreaterEqual(NAME, VAL)       SG_REFLECTION_IMPL_CHECK_PROPERTY_GreaterEqual(NAME, VAL)
#define REFLECTION_CHECK_PROPERTY_Less(NAME, VAL)               SG_REFLECTION_IMPL_CHECK_PROPERTY_Less(NAME, VAL)
#define REFLECTION_CHECK_PROPERTY_LessEqual(NAME, VAL)          SG_REFLECTION_IMPL_CHECK_PROPERTY_LessEqual(NAME, VAL)
#endif

#endif
