#ifndef Reflection_Metaclass_H
#define Reflection_Metaclass_H

#include <Core/Config.h>
#include "Config.h"
#include "Property.h"

namespace sg {
namespace reflection {
//=============================================================================
class Metaclass;
struct MetaclassRegistrator;
class BaseClass;
struct BaseType;
//=============================================================================
void InitMetaclasses();
void RegisterMetaclassesIFN();
Metaclass const* GetMetaclassIFP(Identifier const& iFullClassName);
Metaclass const* GetMetaclass(Identifier const& iFullClassName);
Metaclass const* GetMetaclassIFP(Identifier const& iCurrentNamespace, Identifier const& iClassName);
void ShutdownMetaclasses();
//=============================================================================
typedef Metaclass* (*CreateMetaclassFct) (MetaclassRegistrator* iRegistrator);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct MetaclassInfo
{
    char const*const* classNamespace;
    size_t classNamespaceSize;
    char const* className;
    bool isClass;
    bool isConcrete;
    bool enableListConstruct;
    bool enableStructConstruct;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct MetaclassRegistrator
{
public:
    MetaclassRegistrator(MetaclassRegistrator* iParentRegistrator, CreateMetaclassFct iMetaclassCreator, MetaclassInfo const& iInfo, bool& iIsDeclared)
        : parentRegistrator(iParentRegistrator)
        , metaclassCreator(iMetaclassCreator)
        , info(iInfo)
        , isDeclared(iIsDeclared)
        , metaclass(nullptr)
        , nextRegistrator(firstRegistrator)
    {
        firstRegistrator = this;
        SG_ASSERT(nullptr == info.classNamespace[info.classNamespaceSize]);
        SG_ASSERT_MSG(info.isClass || !info.isConcrete, "isConcrete has no meaning for non-class. As a convention, it is requested to be set at false.");
        SG_ASSERT(!info.isClass || !info.enableListConstruct);
        SG_ASSERT(!info.isClass || info.enableStructConstruct);
        SG_ASSERT(info.enableListConstruct || info.enableStructConstruct);
    }
public:
    MetaclassRegistrator* const parentRegistrator;
    CreateMetaclassFct const metaclassCreator;
    MetaclassInfo const& info;
    bool const& isDeclared;
    Metaclass* metaclass;
    MetaclassRegistrator* nextRegistrator;
public:
    static MetaclassRegistrator* firstRegistrator;
};
//=============================================================================
typedef BaseClass* (*CreateObjectFct) (void);
class Metaclass
{
    SG_NON_COPYABLE(Metaclass)
public:
    Identifier const& FullClassName() const { return m_fullClassName; }
    char const* ClassName() const { return m_registrator->info.className; }
    BaseClass* CreateObject() const;
    IProperty const* GetPropertyIFP(char const* iName) const;
    IProperty const* GetProperty(size_t i) const;
    size_t GetPropertyCount() const;
    void GetObjectPropertyIFP(void const* iObject, char const* iName, refptr<IPrimitiveData>* oValue) const;
    bool SetObjectPropertyROK(void* iObject, char const* iName, IPrimitiveData const* iValue) const;
    bool IsClass() const { return m_registrator->info.isClass; }
    bool IsConcrete() const { return m_registrator->info.isConcrete; }
    bool EnableListConstruct() const { return m_registrator->info.enableListConstruct; }
    bool EnableStructConstruct() const { return m_registrator->info.enableStructConstruct; }
    Metaclass const* GetParentClass() const { SG_ASSERT(nullptr != m_registrator); return nullptr != m_registrator->parentRegistrator ? m_registrator->parentRegistrator->metaclass : nullptr; }
    bool IsBaseOf(Metaclass const* iOther) const;
public:
    Metaclass(MetaclassRegistrator* iRegistrator,
              CreateObjectFct iObjectCreator);
#if ENABLE_REFLECTION_DOCUMENTATION
    void SetDocumentation(char const* iDocumentation);
#endif
    void RegisterProperty(IProperty* iProperty);
private:
    IProperty const* GetPropertyIFP(size_t iIndex, size_t& oIndexInCaller) const;
private:
    MetaclassRegistrator const* const m_registrator;
    CreateObjectFct const m_objectCreator;
    std::vector<refptr<IProperty const> > m_properties;
    Identifier m_fullClassName;
#if ENABLE_REFLECTION_DOCUMENTATION
    char const* m_documentation;
#endif
};
//=============================================================================
}
}

#endif
