#include "stdafx.h"

#include "ObjectDatabaseForwardPopulator.h"

#include "BaseClass.h"
#include "Metaclass.h"

namespace sg {
namespace reflection {
//=============================================================================
ObjectDatabaseForwardPopulator::ObjectDatabaseForwardPopulator(ObjectDatabase* iDatabase)
    : m_stack()
    , m_path()
    , m_database(iDatabase)
    , m_propertyContainsIdentifier(false)
{
}
ObjectDatabaseForwardPopulator::~ObjectDatabaseForwardPopulator()
{
    SG_ASSERT(m_stack.empty());
    SG_ASSERT(m_path.Size() == 0);
    SG_ASSERT(!m_propertyContainsIdentifier);
}
void ObjectDatabaseForwardPopulator::BeginNamespace(char const* iName)
{
    SG_ASSERT(m_stack.empty());
    SG_ASSERT(!m_propertyContainsIdentifier);
    m_path = Identifier(m_path, iName);
}
void ObjectDatabaseForwardPopulator::EndNamespace()
{
    SG_ASSERT(m_stack.empty());
    SG_ASSERT(!m_propertyContainsIdentifier);
    m_path = m_path.ParentNamespace();
}
void ObjectDatabaseForwardPopulator::BeginObject(ObjectVisibility iVisibility, char const* iName, Identifier const& iType)
{
    SG_ASSERT(!m_propertyContainsIdentifier);
    using namespace reflection;
    if(!m_stack.empty())
        BeginValue();
    SG_CODE_FOR_ASSERT(std::string DEBUG_type = iType.AsString());
    Metaclass const* mc = GetMetaclassIFP(m_path, iType);
    if(nullptr == mc)
    {
        // TODO error...
    }
    SG_ASSERT(nullptr != mc);
    BaseClass* object = mc->CreateObject();
    m_stack.emplace_back(object);
    m_path = Identifier(m_path, iName);
    m_database->Add(iVisibility, m_path, object);
}
void ObjectDatabaseForwardPopulator::EndObject()
{
    SG_ASSERT(!m_propertyContainsIdentifier);
    using namespace reflection;
    SG_ASSERT(!m_stack.empty());
    SG_ASSERT(m_stack.back().type == StackNode::Type::Object);
    safeptr<BaseClass> object = m_stack.back().object.get();
    SG_ASSERT(nullptr != object);
    m_stack.pop_back();
    if(!m_stack.empty())
    {
        refptr<IPrimitiveData> data = new PrimitiveData<refptr<BaseClass> >(object.get());
        EndValue(data.get());
    }
    m_path = m_path.ParentNamespace();
}
void ObjectDatabaseForwardPopulator::Property(char const* iName)
{
    SG_ASSERT(!m_stack.empty());
    SG_ASSERT(m_stack.back().type == StackNode::Type::Object || m_stack.back().type == StackNode::Type::NamedList);
    SG_ASSERT(!m_propertyContainsIdentifier || m_stack.back().type != StackNode::Type::Object);
    SG_ASSERT(nullptr == m_stack.back().propertyName);
    m_stack.back().propertyName = iName;
}
void ObjectDatabaseForwardPopulator::Value(bool iValue)
{
    BeginValue();
    refptr<IPrimitiveData> data = new PrimitiveData<bool>(iValue);
    EndValue(data.get());
}
void ObjectDatabaseForwardPopulator::Value(int iValue)
{
    BeginValue();
    refptr<IPrimitiveData> data = new PrimitiveData<i32>(iValue);
    EndValue(data.get());
}
void ObjectDatabaseForwardPopulator::Value(float iValue)
{
    BeginValue();
    refptr<IPrimitiveData> data = new PrimitiveData<float>(iValue);
    EndValue(data.get());
}
void ObjectDatabaseForwardPopulator::Value(char const* iValue)
{
    BeginValue();
    refptr<IPrimitiveData> data = new PrimitiveData<std::string>(iValue);
    EndValue(data.get());
}
void ObjectDatabaseForwardPopulator::Value(nullptr_t)
{
    BeginValue();
    refptr<IPrimitiveData> data = new PrimitiveData<nullptr_t>(nullptr);
    EndValue(data.get());
}
void ObjectDatabaseForwardPopulator::Value(Identifier const& iValue)
{
    BeginValue();
    m_propertyContainsIdentifier = true;
    refptr<IPrimitiveData> data = new PrimitiveData<ObjectReference>(
        ObjectReference(
            m_database.get(),
            m_path,
            iValue));
    EndValue(data.get());
}
void ObjectDatabaseForwardPopulator::BeginList()
{
    BeginValue();
    m_stack.emplace_back(StackNode::Type::List);
    m_stack.back().data = new PrimitiveData<PrimitiveDataList>();
}
void ObjectDatabaseForwardPopulator::EndList()
{
    refptr<IPrimitiveData> data = m_stack.back().data;
    m_stack.pop_back();
    EndValue(data.get());
}
void ObjectDatabaseForwardPopulator::BeginBloc()
{
    BeginValue();
    m_stack.emplace_back(StackNode::Type::NamedList);
    m_stack.back().data = new PrimitiveData<PrimitiveDataNamedList>();
}
void ObjectDatabaseForwardPopulator::EndBloc()
{
    refptr<IPrimitiveData> data = m_stack.back().data;
    m_stack.pop_back();
    EndValue(data.get());
}
void ObjectDatabaseForwardPopulator::BeginValue()
{
    SG_ASSERT(!m_stack.empty());
    SG_ASSERT(nullptr != m_stack.back().propertyName || m_stack.back().type == StackNode::Type::List);
}
void ObjectDatabaseForwardPopulator::EndValue(IPrimitiveData* iData)
{
    SG_ASSERT(iData->IsRefCounted_ForAssert());
    StackNode& backNode = m_stack.back();
    switch(backNode.type)
    {
    case StackNode::Type::Object:
        {
            SG_ASSERT(nullptr != backNode.object);
            SG_ASSERT(nullptr != backNode.propertyName);
            if(!m_propertyContainsIdentifier)
            {
                bool ok = backNode.object->SetPropertyROK(backNode.propertyName, iData);
                SG_ASSERT_AND_UNUSED(ok);
            }
            else
            {
                IProperty const* property = backNode.object->GetMetaclass()->GetPropertyIFP(backNode.propertyName);
                m_database->AddDeferredProperty(m_path, property, iData);
                m_propertyContainsIdentifier = false;
            }
        }
        break;
    case StackNode::Type::List:
        {
            SG_ASSERT(nullptr == backNode.object);
            SG_ASSERT(nullptr == backNode.propertyName);
            PrimitiveData<PrimitiveDataList>* nodeData = checked_cast<PrimitiveData<PrimitiveDataList>*>(backNode.data.get());
            nodeData->GetForWriting().push_back(iData);
        }
        break;
    case StackNode::Type::NamedList:
        {
            SG_ASSERT(nullptr == backNode.object);
            SG_ASSERT(nullptr != backNode.propertyName);
            PrimitiveData<PrimitiveDataNamedList>* nodeData = checked_cast<PrimitiveData<PrimitiveDataNamedList>*>(backNode.data.get());
            nodeData->GetForWriting().emplace_back(backNode.propertyName, iData);
        }
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    backNode.propertyName = nullptr;
}
//=============================================================================
}
}

