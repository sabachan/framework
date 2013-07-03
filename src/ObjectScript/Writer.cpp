#include "stdafx.h"

#include <Core/Config.h>

#if SG_ENABLE_TOOLS

#include "Writer.h"

#include <Core/Assert.h>
#include <Core/Log.h>
#include <Core/PerfLog.h>
#include <Core/SimpleFileReader.h>
#include <Reflection/BaseClass.h>
#include <Reflection/InitShutdown.h>
#include <Reflection/ObjectDatabase.h>
#include <Reflection/PrimitiveData.h>
#include <sstream>
#include <iomanip>

namespace sg {
namespace objectscript {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
char const* TAB = "\t";
//=============================================================================
bool IsDefaultValue(reflection::IPrimitiveData* iData)
{
    switch(iData->GetType())
    {
    case reflection::PrimitiveDataType::Null:
    case reflection::PrimitiveDataType::Boolean:
    case reflection::PrimitiveDataType::Int32:
    case reflection::PrimitiveDataType::UInt32:
    case reflection::PrimitiveDataType::Float:
    case reflection::PrimitiveDataType::String:
    case reflection::PrimitiveDataType::List:
    case reflection::PrimitiveDataType::Object:
        return iData->IsDefault();
    case reflection::PrimitiveDataType::NamedList:
        {
            reflection::PrimitiveDataNamedList list;
            iData->As<reflection::PrimitiveDataNamedList>(&list);
            size_t const size = list.size();
            for(size_t i = 0; i < size; ++i)
            {
                reflection::IPrimitiveData* value = list[i].second.get();
                if(!IsDefaultValue(value))
                    return false;
            }
            return true;
        }
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    SG_ASSERT_NOT_REACHED();
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
class Writer
{
public:
    Writer(reflection::ObjectDatabase const& iObjectDatabase);
    void Write(std::ostringstream& oss);
private:
    void PrintVisibility(std::ostringstream& oss, reflection::ObjectVisibility iVisibility);
    void PrintIdentifierNode(std::ostringstream& oss, reflection::IdentifierNode const& iIdentifierNode);
    void PrintObject(std::ostringstream& oss, size_t tab, reflection::BaseClass const* object);
    void PrintValue(std::ostringstream& oss, size_t tab, reflection::IPrimitiveData const* iData);
private:
    safeptr<reflection::ObjectDatabase const> m_objectDatabase;
    std::unordered_map<void const*, size_t> m_pointerToObjectEntry;
    std::unordered_map<size_t, size_t> m_anonymousReindexing;
};
//=============================================================================
Writer::Writer(reflection::ObjectDatabase const& iObjectDatabase)
    : m_objectDatabase(&iObjectDatabase)
{
    size_t const objectCount = m_objectDatabase->m_objects.size();
    for(size_t i = 0; i < objectCount; ++i)
    {
        reflection::ObjectDatabase::ObjectEntry const& objectEntry = m_objectDatabase->m_objects[i];
        m_pointerToObjectEntry.insert(std::make_pair((void const*)(objectEntry.object.get()), i));
    }
}
//=============================================================================
void Writer::PrintVisibility(std::ostringstream& oss, reflection::ObjectVisibility iVisibility)
{
    switch(iVisibility)
    {
    case reflection::ObjectVisibility::Export:    oss << "export";    break;
    case reflection::ObjectVisibility::Private:   oss << "private";   break;
    case reflection::ObjectVisibility::Protected: oss << "protected"; break;
    case reflection::ObjectVisibility::Public:    oss << "public";    break;
    default: SG_ASSUME_NOT_REACHED();
    }
}
//=============================================================================
void Writer::PrintIdentifierNode(std::ostringstream& oss, reflection::IdentifierNode const& iIdentifierNode)
{
    if(iIdentifierNode.IsAnonymous())
    {
        // Note: this reindexing is used to generate names for anonymous identifers
        // that begin with 0 and use consecutive integers in a file. This is usefull
        // not to generate as much symbols as there may be anonymous indices, as
        // thos symbols are kept almost forever in a FastSymbol database.
        auto f = m_anonymousReindexing.insert(std::make_pair(iIdentifierNode.AnonymousIndex(), m_anonymousReindexing.size()));
        size_t reindexing = f.first->second;
        oss << "_anonymous_" << std::setw(4) << std::setfill('0') << reindexing;
    }
    else
        oss << iIdentifierNode.Symbol().Value();

}
//=============================================================================
void Writer::Write(std::ostringstream& oss)
{
    size_t const objectCount = m_objectDatabase->m_objects.size();
    reflection::Identifier currentNamespace;
    size_t tab = 0;
    for(size_t i = 0; i < objectCount; ++i)
    {
        reflection::ObjectDatabase::ObjectEntry const& objectEntry = m_objectDatabase->m_objects[i];
        reflection::Identifier const& identifier = objectEntry.id;
        reflection::Identifier const& parentNamespace = identifier.ParentNamespace();
        while(!currentNamespace.Contains(parentNamespace))
        {
            --tab;
            for(size_t t = 0; t < tab; ++t) oss << TAB;
            oss << "}" << std::endl;
            currentNamespace = currentNamespace.ParentNamespace();
        }
        for(size_t j = currentNamespace.Size(); j < parentNamespace.Size(); ++j)
        {
            for(size_t t = 0; t < tab; ++t) oss << TAB;
            oss << "namespace ";
            PrintIdentifierNode(oss, parentNamespace[j]);
            oss << " {" << std::endl;
            currentNamespace.PushBack(parentNamespace[j]);
            ++tab;
        }
        reflection::BaseClass const*const object = objectEntry.object.get();
        for(size_t t = 0; t < tab; ++t) oss << TAB;
        PrintVisibility(oss, objectEntry.visibility);
        oss << " ";
        PrintIdentifierNode(oss, identifier.Back());
        oss << " is ";
        PrintObject(oss, tab, object);
    }
    while(!currentNamespace.Empty())
    {
        --tab;
        for(size_t t = 0; t < tab; ++t) oss << TAB;
        oss << "}" << std::endl;
        currentNamespace = currentNamespace.ParentNamespace();
    }
}
//=============================================================================
void Writer::PrintObject(std::ostringstream& oss, size_t tab, reflection::BaseClass const* object)
{
    reflection::Metaclass const*const mc = object->GetMetaclass();
    oss << mc->FullClassName().AsString() << std::endl;
    for(size_t t = 0; t < tab; ++t) oss << TAB;
    oss << "{" << std::endl;
    ++tab;
    size_t const propCount = mc->GetPropertyCount();
    for(size_t j = 0; j < propCount; ++j)
    {
        reflection::IProperty const*const prop = mc->GetProperty(j);
        refptr<reflection::IPrimitiveData> data;
        object->GetProperty(prop, &data);
        if(!IsDefaultValue(data.get()))
        {
            for(size_t t = 0; t < tab; ++t) oss << TAB;
            oss << prop->Name() << ": ";
            PrintValue(oss, tab, data.get());
            oss << std::endl;
        }
    }
    --tab;
    for(size_t t = 0; t < tab; ++t) oss << TAB;
    oss << "}" << std::endl;
}
//=============================================================================
void Writer::PrintValue(std::ostringstream& oss, size_t tab, reflection::IPrimitiveData const* iData)
{
    switch(iData->GetType())
    {
    case reflection::PrimitiveDataType::Null: oss << "null"; break;
    case reflection::PrimitiveDataType::Boolean: oss << (iData->As<bool>() ? "true" : "false"); break;
    case reflection::PrimitiveDataType::Int32: oss << iData->As<i32>(); break;
    case reflection::PrimitiveDataType::UInt32: oss << iData->As<u32>(); break;
    case reflection::PrimitiveDataType::Float: oss << iData->As<float>(); break;
    case reflection::PrimitiveDataType::String: oss << '\"' << iData->As<std::string>() << '\"'; break;
    case reflection::PrimitiveDataType::List:
        {
            reflection::PrimitiveDataList list;
            iData->As<reflection::PrimitiveDataList>(&list);
            oss << "[" << std::endl;
            ++tab;
            for(size_t i = 0; i < list.size(); ++i)
            {
                if(i != 0)
                    oss << "," << std::endl;
                for(size_t t = 0; t < tab; ++t) oss << TAB;
                PrintValue(oss, tab, list[i].get());
            }
            oss << std::endl;
            --tab;
            for(size_t t = 0; t < tab; ++t) oss << TAB;
            oss << "]";
        }
        break;
    case reflection::PrimitiveDataType::NamedList:
        {
            reflection::PrimitiveDataNamedList list;
            iData->As<reflection::PrimitiveDataNamedList>(&list);
            oss << "{";
            ++tab;
            for(size_t i = 0; i < list.size(); ++i)
            {
                oss << std::endl;
                for(size_t t = 0; t < tab; ++t) oss << TAB;
                oss << list[i].first << ": ";
                PrintValue(oss, tab, list[i].second.get());
            }
            oss << std::endl;
            --tab;
            for(size_t t = 0; t < tab; ++t) oss << TAB;
            oss << "}";
        }
        break;
    case reflection::PrimitiveDataType::Object:
        {
            refptr<reflection::BaseClass> object;
            iData->As<refptr<reflection::BaseClass> >(&object);
            SG_ASSERT(nullptr != object);
            auto f = m_pointerToObjectEntry.find(object.get());
            if(f == m_pointerToObjectEntry.end())
            {
                SG_ASSERT_MSG(f != m_pointerToObjectEntry.end(), "referenced object has not been found in database.");
                // TODO: consider it as a local private object ?
            }
            else
            {
                reflection::ObjectDatabase::ObjectEntry const& objectEntry = m_objectDatabase->m_objects[f->second];
                reflection::Identifier identifier = objectEntry.id;
                size_t const size = identifier.Size();
                for(size_t i = 0; i < size; ++i)
                {
                    oss << "::";
                    PrintIdentifierNode(oss, identifier[i]);
                }
            }
        }
        break;
    case reflection::PrimitiveDataType::ObjectReference:
        SG_ASSERT_NOT_REACHED();
    default:
        SG_ASSERT_NOT_REACHED();
    }
}
//=============================================================================
void WriteObjectScript(std::string& oFileContent, reflection::ObjectDatabase const& iObjectDatabase)
{
    Writer writer(iObjectDatabase);
    std::ostringstream oss;
    writer.Write(oss);
    oFileContent = oss.str();
}
//=============================================================================
}
}
#endif
