#include "stdafx.h"

#include "Intrinsics.h"
#include <Reflection/BaseClass.h>
#include <algorithm>

namespace sg {
namespace objectscript {
//=============================================================================
namespace {
//=============================================================================
bool Intrinsic_IsSameType(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage)
{
    SG_ASSERT(nullptr == oResult);
    if(2 != iArgs.size())
    {
        oErrorMessage = "incorrect number of arguments";
        return false;
    }
    bool const isSameType = iArgs[0]->GetType() == iArgs[1]->GetType();
    oResult = new reflection::PrimitiveData<bool>(isSameType);
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Intrinsic_ListLength(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage)
{
    SG_ASSERT(nullptr == oResult);
    if(1 != iArgs.size())
    {
        oErrorMessage = "incorrect number of arguments";
        return false;
    }
    switch(iArgs[0]->GetType())
    {
    case reflection::PrimitiveDataType::List:
    {
        reflection::PrimitiveDataList const& list = iArgs[0]->As<reflection::PrimitiveDataList>();
        u32 const length = checked_numcastable(list.size());
        oResult = new reflection::PrimitiveData<u32>(length);
        return true;
    }
    default:
    {
        oErrorMessage = "incorrect type for arguments";
        return false;
    }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Intrinsic_TypeNameOf(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage)
{
    SG_ASSERT(nullptr == oResult);
    if(1 != iArgs.size())
    {
        oErrorMessage = "incorrect number of arguments";
        return false;
    }
    reflection::PrimitiveDataType type = iArgs[0]->GetType();
    char const* typeName = GetPrimitiveDataTypeName(type);
    oResult = new reflection::PrimitiveData<std::string>(typeName);
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

//=============================================================================
struct IntrinsicTrait
{
    char const* name;
    IntrinsicFct fct;
};
IntrinsicTrait intrinsicTraits[] = {
    { "is_same_type", Intrinsic_IsSameType },
    { "list_length", Intrinsic_ListLength },
    { "type_name_of", Intrinsic_TypeNameOf },
};
//=============================================================================
}
//=============================================================================
IntrinsicFct GetIntrinsic(char const* iName)
{
    auto const begin = intrinsicTraits;
    auto const end = intrinsicTraits + SG_ARRAYSIZE(intrinsicTraits);
    auto cmp = [](IntrinsicTrait const& a, IntrinsicTrait const& b) { return strcmp(a.name, b.name) < 0; };
    SG_ASSERT(std::is_sorted(begin, end, cmp));
    auto f = std::lower_bound(begin, end, iName, [](IntrinsicTrait const& a, char const* iName) { return strcmp(a.name, iName) < 0; });
    if(strcmp(f->name, iName) == 0)
        return f->fct;
    else
        return nullptr;
}
//=============================================================================
}
}

