#include "stdafx.h"
#include "PrimitiveData.h"

#include "BaseClass.h"
#if SG_ENABLE_TOOLS
#include <sstream>
#endif

namespace sg {
namespace reflection {
//=============================================================================
void IPrimitiveData::CopyTo(refptr<IPrimitiveData>& oCopy) const
{
    switch(GetType())
    {
    case PrimitiveDataType::Null: oCopy = new PrimitiveData<nullptr_t>(); return;
    case PrimitiveDataType::Boolean: oCopy = new PrimitiveData<bool>(As<bool>()); return;
    case PrimitiveDataType::Int32: oCopy = new PrimitiveData<i32>(As<i32>()); return;
    case PrimitiveDataType::UInt32: oCopy = new PrimitiveData<u32>(As<u32>()); return;
    case PrimitiveDataType::Float: oCopy = new PrimitiveData<float>(As<float>()); return;
    case PrimitiveDataType::String: oCopy = new PrimitiveData<std::string>(As<std::string>()); return;
    case PrimitiveDataType::List:
        {
            PrimitiveData<PrimitiveDataList>* dataListCopy = new PrimitiveData<PrimitiveDataList>();
            PrimitiveData<PrimitiveDataList> const* dataList = checked_cast<PrimitiveData<PrimitiveDataList> const*>(this);
            PrimitiveDataList& listCopy = dataListCopy->GetForWriting();
            PrimitiveDataList const& list = dataList->Get();
            size_t const size = list.size();
            listCopy.reserve(size);
            for(size_t i = 0; i < size; ++i)
            {
                listCopy.emplace_back();
                list[i]->CopyTo(listCopy[i]);
            }
            oCopy = dataListCopy;
        }
        return;
    case PrimitiveDataType::NamedList:
        {
            PrimitiveData<PrimitiveDataNamedList>* dataListCopy = new PrimitiveData<PrimitiveDataNamedList>();
            PrimitiveData<PrimitiveDataNamedList> const* dataList = checked_cast<PrimitiveData<PrimitiveDataNamedList> const*>(this);
            PrimitiveDataNamedList& listCopy = dataListCopy->GetForWriting();
            PrimitiveDataNamedList const& list = dataList->Get();
            size_t const size = list.size();
            listCopy.reserve(size);
            for(size_t i = 0; i < size; ++i)
            {
                listCopy.emplace_back();
                listCopy[i].first = list[i].first;
                list[i].second->CopyTo(listCopy[i].second);
            }
            oCopy = dataListCopy;
        }
        return;
    case PrimitiveDataType::Object:
        SG_ASSERT_NOT_IMPLEMENTED();
        return;
    case PrimitiveDataType::ObjectReference:
        oCopy = new PrimitiveData<ObjectReference>(As<ObjectReference>()); return;
    default:
        SG_ASSERT_NOT_REACHED();
    }
}
//=============================================================================
bool SetToNullROK(refptr<BaseClass>* oValue)
{
    SG_ASSERT(nullptr != oValue);
    *oValue = nullptr;
    return true;
}
//=============================================================================
bool DoesContainObjectReference(IPrimitiveData* iData)
{
    switch(iData->GetType())
    {
    case PrimitiveDataType::Null:
    case PrimitiveDataType::Boolean:
    case PrimitiveDataType::Int32:
    case PrimitiveDataType::UInt32:
    case PrimitiveDataType::Float:
    case PrimitiveDataType::String:
        return false;
    case PrimitiveDataType::List:
        {
            PrimitiveData<PrimitiveDataList> const* dataList = checked_cast<PrimitiveData<PrimitiveDataList> const*>(iData);
            PrimitiveDataList const& list = dataList->Get();
            size_t const size = list.size();
            for(size_t i = 0; i < size; ++i)
            {
                if(DoesContainObjectReference(list[i].get()))
                    return true;
            }
            return false;
        }
    case PrimitiveDataType::NamedList:
        {
            PrimitiveData<PrimitiveDataNamedList> const* dataList = checked_cast<PrimitiveData<PrimitiveDataNamedList> const*>(iData);
            PrimitiveDataNamedList const& list = dataList->Get();
            size_t const size = list.size();
            for(size_t i = 0; i < size; ++i)
            {
                if(DoesContainObjectReference(list[i].second.get()))
                    return true;
            }
            return false;
        }
    case PrimitiveDataType::Object:
        return false;
    case PrimitiveDataType::ObjectReference:
        return true;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    SG_ASSERT_NOT_REACHED();
    return false;
}
//=============================================================================
#if SG_ENABLE_TOOLS
std::string ToString(IPrimitiveData* iData)
{
    std::ostringstream oss;
    switch(iData->GetType())
    {
    case PrimitiveDataType::Null: return "null";
    case PrimitiveDataType::Boolean: return iData->As<bool>() ? "true" : "false";
    case PrimitiveDataType::Int32: oss << iData->As<i32>(); return oss.str();
    case PrimitiveDataType::UInt32: oss << iData->As<u32>(); return oss.str();
    case PrimitiveDataType::Float: oss << iData->As<float>(); return oss.str();
    case PrimitiveDataType::String: return iData->As<std::string>();
    case PrimitiveDataType::List:
        {
            PrimitiveDataList list;
            iData->As<PrimitiveDataList>(&list);
            oss << "[";
            for(size_t i = 0; i < list.size(); ++i)
            {
                if(i != 0)
                    oss << ", ";
                oss << ToString(list[i].get());
            }
            oss << "]";
            return oss.str();
        }
    case PrimitiveDataType::NamedList:
        {
            PrimitiveDataNamedList list;
            iData->As<PrimitiveDataNamedList>(&list);
            oss << "{";
            for(size_t i = 0; i < list.size(); ++i)
            {
                oss << std::endl;
                oss << list[i].first << ": ";
                oss << ToString(list[i].second.get());
            }
            oss << std::endl << " }";
            return oss.str();
        }
    case PrimitiveDataType::Object: return "<Object>";
    case PrimitiveDataType::ObjectReference: return iData->As<ObjectReference>().AsString();
    default:
        SG_ASSERT_NOT_REACHED();
    }
    return std::string();
}
#endif
//=============================================================================
}
}
