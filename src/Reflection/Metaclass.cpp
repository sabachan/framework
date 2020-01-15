#include "stdafx.h"

#include "Metaclass.h"

#include "BaseClass.h"
#include <Core/Assert.h>
#include <Core/Log.h>
#include <Core/PerfLog.h>
#include <Core/Platform.h>
#include <Core/StringFormat.h>
#if SG_PLATFORM_IS_WIN
#include <Core/WinUtils.h>
#endif
#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace sg {
namespace reflection {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class MetaclassDatabase
{
public:
    void RegisterMetaclass(Metaclass const* mc);
    Metaclass const* GetMetaclassIFP(Identifier const& iFullClassName) const;
    Metaclass const* GetMetaclassIFP(Identifier const& iCurrentNamespace, Identifier const& iClassName) const;

private:
    std::vector<Metaclass const*> m_metaclasses;
    std::unordered_multimap<IdentifierSymbol, size_t> m_lastSymbolToMetaclasses;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
MetaclassDatabase* s_metaclassDatabase = nullptr;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void MetaclassDatabase::RegisterMetaclass(Metaclass const* mc)
{
    Identifier const& fullClassName = mc->FullClassName();
    SG_ASSERT(fullClassName.IsAbsolute());
    SG_ASSERT_MSG(nullptr == GetMetaclassIFP(fullClassName), "a metaclass with same class name already exists");
    IdentifierNode const lastNode = fullClassName.Back();
    SG_ASSERT(!lastNode.IsAnonymous());
    m_lastSymbolToMetaclasses.emplace(lastNode.Symbol(), m_metaclasses.size());
    m_metaclasses.push_back(mc);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Metaclass const* MetaclassDatabase::GetMetaclassIFP(Identifier const& iFullClassName) const
{
    SG_ASSERT(iFullClassName.IsAbsolute());

    size_t const iIdSize = iFullClassName.Size();
    IdentifierNode const lastNode = iFullClassName.Back();
    SG_ASSERT(!lastNode.IsAnonymous());
    auto begin_end = m_lastSymbolToMetaclasses.equal_range(lastNode.Symbol());
    if(begin_end.first == m_lastSymbolToMetaclasses.end())
        return nullptr;
    for(auto it = begin_end.first; it != begin_end.second; ++it)
    {
        Metaclass const* mc = m_metaclasses[it->second];
        Identifier const& id = mc->FullClassName();
        SG_ASSERT(id.IsAbsolute());
        if(id.Size() != iIdSize)
            continue;
        bool match = true;
        SG_ASSERT(id[iIdSize-1] == iFullClassName[iIdSize-1]);
        for(size_t i = 0; i < iIdSize-1; ++i)
        {
            if(id[i] != iFullClassName[i])
            {
                match = false;
                break;
            }
        }
        if(match)
            return mc;
    }
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Metaclass const* MetaclassDatabase::GetMetaclassIFP(Identifier const& iCurrentNamespace, Identifier const& iClassName) const
{
    SG_ASSERT(iCurrentNamespace.IsAbsolute());
    SG_CODE_FOR_ASSERT(std::string DEBUG_iClassName = iClassName.AsString();)
    if(iClassName.IsAbsolute())
        return GetMetaclassIFP(iClassName);

    size_t const iClassNameSize = iClassName.Size();
    size_t const namespaceSize = iCurrentNamespace.Size();
    int bestPrefixMatchingSize = -1;
    size_t bestAllMatchingSize = iClassNameSize;
    size_t bestMetaclassIndex = all_ones;
    IdentifierNode const lastNode = iClassName.Back();
    SG_ASSERT(!lastNode.IsAnonymous());
    auto begin_end = m_lastSymbolToMetaclasses.equal_range(lastNode.Symbol());
    for(auto it = begin_end.first; it != begin_end.second; ++it)
    {
        Metaclass const* mc = m_metaclasses[it->second];
        Identifier const className = mc->FullClassName();
        SG_ASSERT(className.IsAbsolute());
        size_t const classNameSize = className.Size();
        SG_ASSERT(className[classNameSize-1] == iClassName[iClassNameSize-1]);
        if(classNameSize < iClassNameSize)
            continue;
        bool matchSuffix = true;
        for(size_t i = 2; i <= iClassNameSize; ++i)
        {
            SG_ASSERT(!iClassName[iClassNameSize-i].IsAnonymous());
            if(className[classNameSize-i] != iClassName[iClassNameSize-i])
            {
                matchSuffix = false;
                break;
            }
        }
        if(!matchSuffix)
            continue;
        size_t prefixMatchingSize = std::min(namespaceSize, classNameSize);
        for(size_t i = 0; i < prefixMatchingSize; ++i)
        {
            if(className[i] != iCurrentNamespace[i])
            {
                prefixMatchingSize = i;
                break;
            }
        }
        if(prefixMatchingSize + iClassNameSize < classNameSize)
            continue;
        if((int)prefixMatchingSize > bestPrefixMatchingSize)
        {
            bestPrefixMatchingSize = (int)prefixMatchingSize;
            bestAllMatchingSize = classNameSize;
            bestMetaclassIndex = it->second;
        }
        else if((int)classNameSize > bestAllMatchingSize)
        {
            bestAllMatchingSize = (int)classNameSize;
            bestMetaclassIndex = it->second;
        }
    }
    if(-1 != bestMetaclassIndex)
        return m_metaclasses[bestMetaclassIndex];
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
MetaclassRegistrator* MetaclassRegistrator::firstRegistrator = nullptr;
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void InitMetaclasses()
{
    SG_ASSERT_MSG(nullptr == s_metaclassDatabase, "reflection is already initialized");
    SG_ASSERT_MSG(IdentifierSymbol::IsInit_ForAssert(), "IdentifierSymbol must be initialized before Metaclasses");
    s_metaclassDatabase = new MetaclassDatabase();

    RegisterMetaclassesIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RegisterMetaclassesIFN()
{
    SG_SIMPLE_CPU_PERF_LOG_SCOPE("Register metaclasses");
    SG_ASSERT_MSG(nullptr != s_metaclassDatabase, "reflection has not been initialized");
#if SG_ENABLE_ASSERT
    ArrayList<MetaclassRegistrator const*> undeclaredRegistrators;
#endif
    MetaclassRegistrator* registrator = MetaclassRegistrator::firstRegistrator;
    while(nullptr != registrator)
    {
        SG_ASSERT(nullptr != registrator->metaclassCreator);
        SG_ASSERT(nullptr != registrator->parentRegistrator || 0 == strcmp(registrator->info.className, "BaseType"));
        if(nullptr == registrator->metaclass)
        {
#if SG_ENABLE_ASSERT
            if(registrator->info.isClass && registrator->info.isConcrete && !registrator->isDeclared)
            {
                undeclaredRegistrators.EmplaceBack(registrator);
            }
#endif
            Metaclass* mc = registrator->metaclassCreator(registrator);
            SG_ASSERT(nullptr != mc);
            registrator->metaclass = mc;
            s_metaclassDatabase->RegisterMetaclass(mc);
        }
        registrator = registrator->nextRegistrator;
    }
#if SG_ENABLE_ASSERT
    if(!undeclaredRegistrators.Empty())
    {
        std::ostringstream oss;
        oss << "The following metaclasses have not been declared:" << std::endl;
        for(auto const* it : undeclaredRegistrators)
        {
            oss << "    - ";
            for_range(size_t, i, 0, it->info.classNamespaceSize)
                oss << it->info.classNamespace[i] << "::";
            oss << it->info.className << std::endl;
        }
        oss << std::endl;
        oss << "Please declare them in their respective DeclareMetaclasses.h and check for forgotten ones." << std::endl;
        SG_LOG_WARNING("Reflection", oss.str());
#if SG_PLATFORM_IS_WIN
        winutils::ShowModalMessageOK(undeclaredRegistrators.Size() == 1 ? "Undeclared Metaclass" : "Undeclared Metaclasses", oss.str().c_str());
#else
        SG_ASSERT_NOT_IMPLEMENTED();
#endif
    }
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Metaclass const* GetMetaclassIFP(Identifier const& iFullClassName)
{
    SG_ASSERT_MSG(nullptr != s_metaclassDatabase, "reflection has not been initialized");
    Metaclass const* mc = s_metaclassDatabase->GetMetaclassIFP(iFullClassName);
    return mc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Metaclass const* GetMetaclass(Identifier const& iFullClassName)
{
    SG_ASSERT_MSG(nullptr != s_metaclassDatabase, "reflection has not been initialized");
    Metaclass const* mc = s_metaclassDatabase->GetMetaclassIFP(iFullClassName);
    SG_ASSERT(nullptr != mc);
    return mc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Metaclass const* GetMetaclassIFP(Identifier const& iCurrentNamespace, Identifier const& iClassName)
{
    SG_ASSERT_MSG(nullptr != s_metaclassDatabase, "reflection has not been initialized");
    Metaclass const* mc = s_metaclassDatabase->GetMetaclassIFP(iCurrentNamespace, iClassName);
    return mc;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShutdownMetaclasses()
{
    SG_ASSERT_MSG(nullptr != s_metaclassDatabase, "reflection has not been initialized");
    MetaclassRegistrator* registrator = MetaclassRegistrator::firstRegistrator;
    while(nullptr != registrator)
    {
        registrator->metaclass = nullptr;
        registrator = registrator->nextRegistrator;
    }
    delete s_metaclassDatabase;
    s_metaclassDatabase = nullptr;
}
//=============================================================================
Metaclass::Metaclass(MetaclassRegistrator* iRegistrator,
                     CreateObjectFct iObjectCreator)
    : m_registrator(iRegistrator)
    , m_objectCreator(iObjectCreator)
    , m_fullClassName(Identifier::Mode::Absolute)
#if ENABLE_REFLECTION_DOCUMENTATION
    , m_documentation(nullptr)
#endif
{
    for(size_t i = 0; i < m_registrator->info.classNamespaceSize; ++i)
    {
        m_fullClassName.PushBack(m_registrator->info.classNamespace[i]);
    }
    m_fullClassName.PushBack(m_registrator->info.className);
    SG_ASSERT(nullptr != m_registrator);
    SG_ASSERT(m_registrator->info.isClass || !m_registrator->info.isConcrete);
    SG_ASSERT(nullptr == m_objectCreator || m_registrator->info.isConcrete);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_DOCUMENTATION
void Metaclass::SetDocumentation(char const* iDocumentation)
{
    SG_ASSERT(nullptr == m_documentation);
    SG_ASSERT(nullptr != iDocumentation);
    m_documentation = iDocumentation;
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Metaclass::RegisterProperty(IProperty* iProperty)
{
    m_properties.emplace_back(iProperty);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckPropertyIsDefaulted(BaseClass* iObject, std::vector<std::string>& iPropertyPath, reflection::IPrimitiveData* iData, std::vector<std::vector<std::string> >& oUndefaultedProperties)
{
    switch(iData->GetType())
    {
    case PrimitiveDataType::Null:
    case PrimitiveDataType::Boolean:
    case PrimitiveDataType::Int32:
    case PrimitiveDataType::UInt32:
    case PrimitiveDataType::Float:
    case PrimitiveDataType::String:
    case PrimitiveDataType::Object:
        if(!iData->IsDefault())
            oUndefaultedProperties.push_back(iPropertyPath);
        break;
    case PrimitiveDataType::List:
        // Pragmatism: PrimitiveDataList can be used to represent array properties.
        // In that case, the default value is a list of the same size as the array
        // containing only default values.
        // Of course, a vector property, for instance, is expected to be defaulted
        // to the empty vector. Some non-default vectors may then remain undetected.
        // However, I think the risk due to such non-detection is low.
        if(!iData->IsDefault())
        {
            std::string propName;
            using std::swap;
            swap(propName, iPropertyPath.back());
            iPropertyPath.pop_back();
            PrimitiveDataList list;
            iData->As<PrimitiveDataList>(&list);
            size_t const size = list.size();
            for(size_t i = 0; i < size; ++i)
            {
                IPrimitiveData* value = list[i].get();
                iPropertyPath.emplace_back(Format("%0[%1]", propName, i));
                CheckPropertyIsDefaulted(iObject, iPropertyPath, value, oUndefaultedProperties);
                iPropertyPath.pop_back();
            }
            iPropertyPath.emplace_back(propName);
        }
        break;
    case PrimitiveDataType::NamedList:
        {
            PrimitiveDataNamedList list;
            iData->As<PrimitiveDataNamedList>(&list);
            size_t const size = list.size();
            for(size_t i = 0; i < size; ++i)
            {
                IPrimitiveData* value = list[i].second.get();
                iPropertyPath.emplace_back(list[i].first);
                CheckPropertyIsDefaulted(iObject, iPropertyPath, value, oUndefaultedProperties);
                iPropertyPath.pop_back();
            }
        }
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckPropertiesAreDefaulted(BaseClass* iObject)
{
    // Note: Maybe can we better identify a class than with its classname string adress
    static std::unordered_set<char const*> alreadyUndefaultedClasseNames;
    SG_ASSERT(nullptr != iObject);
    Metaclass const* mc = iObject->GetMetaclass();
    SG_ASSERT(mc->IsClass());
    if(alreadyUndefaultedClasseNames.find(mc->ClassName()) == alreadyUndefaultedClasseNames.end())
    {
        std::vector<std::vector<std::string> > undefaultedProperties;
        std::vector<std::string> duplicateNameProperties;
        std::vector<std::string> propertyPath;
        size_t const propCount = mc->GetPropertyCount();
        for(size_t j = 0; j < propCount; ++j)
        {
            IProperty const*const prop = mc->GetProperty(j);
            IProperty const*const propByName = mc->GetPropertyIFP(prop->Name());
            if(propByName != prop)
            {
                duplicateNameProperties.emplace_back(prop->Name());
                continue;
            }
            refptr<IPrimitiveData> data;
            iObject->GetProperty(prop, &data);
            SG_ASSERT(propertyPath.empty());
            propertyPath.emplace_back(prop->Name());
            CheckPropertyIsDefaulted(iObject, propertyPath, data.get(), undefaultedProperties);
            propertyPath.pop_back();
            SG_ASSERT(propertyPath.empty());
        }
        if(!undefaultedProperties.empty() || !duplicateNameProperties.empty())
        {
            alreadyUndefaultedClasseNames.insert(mc->ClassName());
            if(!duplicateNameProperties.empty())
            {
                std::ostringstream oss;
                oss << "Class " << mc->ClassName() << " has properties with conflicting names!" << std::endl;
                oss << std::endl;
                oss << "Duplicate property names are:" << std::endl;
                size_t const duplicateNamePropertyCount = duplicateNameProperties.size();
                for(size_t i = 0; i < duplicateNamePropertyCount; ++i)
                {
                    oss << "    - " << mc->ClassName() << "::";
                    oss << duplicateNameProperties[i];
                    oss << std::endl;
                }
                oss << std::endl;
                oss << "This error is almost ignorable but, even so, should be fixed." << std::endl;
                std::ostringstream osstitle;
                osstitle << mc->ClassName() << " has properties with conflicting names!";
#if SG_PLATFORM_IS_WIN
                winutils::ShowModalMessageOK(osstitle.str().c_str(), oss.str().c_str());
#else
                SG_ASSERT_NOT_IMPLEMENTED();
#endif
            }
            if(!undefaultedProperties.empty())
            {
                std::ostringstream oss;
                oss << "An object of class " << mc->ClassName() << " has some of its properties not set to default values after construction!" << std::endl;
                oss << "All properties must be set to default value in constructor." << std::endl;
                oss << std::endl;
                oss << "Undefaulted properties are:" << std::endl;
                size_t const undefaultedPropCount = undefaultedProperties.size();
                for(size_t i = 0; i < undefaultedPropCount; ++i)
                {
                    oss << "    - " << mc->ClassName() << "::";
                    size_t const pathSize = undefaultedProperties[i].size();
                    for(size_t j = 0; j < pathSize; ++j)
                    {
                        if(j != 0)
                            oss << ".";
                        oss << undefaultedProperties[i][j];
                    }
                    oss << std::endl;
                }
                oss << std::endl;
                oss << "This error is almost ignorable but, even so, should be fixed." << std::endl;
                std::ostringstream osstitle;
                osstitle << mc->ClassName() << " object not defaulted!";
#if SG_PLATFORM_IS_WIN
                winutils::ShowModalMessageOK(osstitle.str().c_str(), oss.str().c_str());
#else
                SG_ASSERT_NOT_IMPLEMENTED();
#endif
            }
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BaseClass* Metaclass::CreateObject() const
{
    SG_ASSERT(m_registrator->info.isClass);
    SG_ASSERT(m_registrator->info.isConcrete);
    SG_ASSERT(nullptr != m_objectCreator);
    BaseClass* object = m_objectCreator();
    SG_ASSERT(nullptr != object);
#if SG_ENABLE_ASSERT
    CheckPropertiesAreDefaulted(object);
#endif
    return object;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IProperty const* Metaclass::GetPropertyIFP(char const* iName) const
{
    for(auto const& p : m_properties)
    {
        if(0 == strcmp(iName, p->Name()))
            return p.get();
    }
    auto parentRegistrator = m_registrator->parentRegistrator;
    if(nullptr != parentRegistrator)
        return parentRegistrator->metaclass->GetPropertyIFP(iName);
    else
        return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IProperty const* Metaclass::GetProperty(size_t i) const
{
    size_t dummyindex = 0;
    IProperty const* prop = GetPropertyIFP(i, dummyindex);
    SG_ASSERT_MSG(nullptr != prop, "Index out of bounds!");
    return prop;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IProperty const* Metaclass::GetPropertyIFP(size_t iIndex, size_t& oIndexInCaller) const
{
    size_t index = iIndex;
    auto parentRegistrator = m_registrator->parentRegistrator;
    IProperty const* prop = nullptr;
    if(nullptr != parentRegistrator)
        prop = parentRegistrator->metaclass->GetPropertyIFP(iIndex, index);
    if(nullptr != prop)
        return prop;
    oIndexInCaller = index-m_properties.size();
    if(index < m_properties.size())
        return m_properties[index].get();
    else
        return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t Metaclass::GetPropertyCount() const
{
    size_t count = m_properties.size();
    auto parentRegistrator = m_registrator->parentRegistrator;
    if(nullptr != parentRegistrator)
        count += parentRegistrator->metaclass->GetPropertyCount();
    return count;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Metaclass::GetObjectPropertyIFP(void const* iObject, char const* iName, refptr<IPrimitiveData>* oValue) const
{
    // Note: as BaseClass has a virtual table, adresses of a BaseClass object
    // and the same viewed as BaseType are not equals.
    SG_ASSERT_MSG(!m_registrator->info.isClass, "GetObjectPropertyIFP is only usable on non virtual objects. Please use BaseClass::GetPropertyIFP instead.");
    SG_ASSERT(nullptr != oValue);
    IProperty const* property = GetPropertyIFP(iName);
    if(nullptr == property)
        *oValue = nullptr;
    else
        property->Get(iObject, oValue);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Metaclass::SetObjectPropertyROK(void* iObject, char const* iName, IPrimitiveData const* iValue) const
{
    // Note: as BaseClass has a virtual table, adresses of a BaseClass object
    // and the same viewed as BaseType are not equals.
    SG_ASSERT_MSG(!m_registrator->info.isClass, "SetObjectPropertyROK is only usable on non virtual object. Please use BaseClass::SetPropertyROK instead.");
    IProperty const* property = GetPropertyIFP(iName);
    if(nullptr == property)
        return nullptr;
    return property->SetROK(iObject, iValue);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Metaclass::IsBaseOf(Metaclass const* iOther) const
{
    Metaclass const* mc = iOther;
    while(nullptr != mc)
    {
        if(mc == this)
            return true;
        mc = mc->GetParentClass();
    }
    return false;
}
//=============================================================================
}
}
