#include "stdafx.h"

#include "Intrinsics.h"
#include <Core/Log.h>
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
bool Intrinsic_IsNumeric(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage)
{
    SG_ASSERT(nullptr == oResult);
    if(1 != iArgs.size())
    {
        oErrorMessage = "incorrect number of arguments";
        return false;
    }
    bool const isNumeric = GetInfo(iArgs[0]->GetType()).isNumeric;
    oResult = new reflection::PrimitiveData<bool>(isNumeric);
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Intrinsic_IsInteger(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage)
{
    SG_ASSERT(nullptr == oResult);
    if(1 != iArgs.size())
    {
        oErrorMessage = "incorrect number of arguments";
        return false;
    }
    bool const isInteger = GetInfo(iArgs[0]->GetType()).isInteger;
    oResult = new reflection::PrimitiveData<bool>(isInteger);
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Intrinsic_Print(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage)
{
    SG_ASSERT_AND_UNUSED(nullptr == oResult);
    if(1 != iArgs.size())
    {
        oErrorMessage = "incorrect number of arguments";
        return false;
    }
    std::string str;
    bool ok = iArgs[0]->AsROK(&str);
    if(!ok)
    {
        oErrorMessage = "incorrect type for argument (expected is string)";
        return false;
    }
#if SG_ENABLE_LOG
    SG_LOG_INFO("ObjectScript/Output", str);
#endif
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename F>
bool Intrinsic_GenericCall_out_Float_in_Float(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage, F const& f)
{
    SG_ASSERT(nullptr == oResult);
    if(1 != iArgs.size())
    {
        oErrorMessage = "incorrect number of arguments (should have 1)";
        return false;
    }

    float in;
    bool const ok = iArgs[0]->AsROK(&in);
    if(!ok)
    {
        oErrorMessage = "incorrect type for argument 0 (should be float)";
        return false;
    }

    float const out = f(in);
    oResult = new reflection::PrimitiveData<float>(out);
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename F>
bool Intrinsic_GenericCall_out_Float_in_Float_Float(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage, F const& f)
{
    SG_ASSERT(nullptr == oResult);
    if(2 != iArgs.size())
    {
        oErrorMessage = "incorrect number of arguments (should have 2)";
        return false;
    }

    float in0;
    bool ok = iArgs[0]->AsROK(&in0);
    if(!ok)
    {
        oErrorMessage = "incorrect type for argument 0 (should be float)";
        return false;
    }
    float in1;
    ok = iArgs[0]->AsROK(&in1);
    if(!ok)
    {
        oErrorMessage = "incorrect type for argument 1 (should be float)";
        return false;
    }

    float const out = f(in0, in1);
    oResult = new reflection::PrimitiveData<float>(out);
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define DEFINE_INTRINSIC_out_Float_in_Float(NAME, LAMBDA) \
    bool Intrinsic_##NAME(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage) \
    { return Intrinsic_GenericCall_out_Float_in_Float(oResult, iArgs, oErrorMessage, LAMBDA); }

DEFINE_INTRINSIC_out_Float_in_Float(cos,        ([](float x){ return cos(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(sin,        ([](float x){ return sin(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(tan,        ([](float x){ return tan(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(acos,       ([](float x){ return acos(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(asin,       ([](float x){ return asin(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(atan,       ([](float x){ return atan(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(exp,        ([](float x){ return exp(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(log,        ([](float x){ return log(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(sqrt,       ([](float x){ return sqrt(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(isfinite,   ([](float x){ return isfinite(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(isinf,      ([](float x){ return isinf(x); }) )
DEFINE_INTRINSIC_out_Float_in_Float(isnan,      ([](float x){ return isnan(x); }) )

#undef DEFINE_INTRINSIC_out_Float_in_Float
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define DEFINE_INTRINSIC_out_Float_in_Float_Float(NAME, LAMBDA) \
    bool Intrinsic_##NAME(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage) \
    { return Intrinsic_GenericCall_out_Float_in_Float_Float(oResult, iArgs, oErrorMessage, LAMBDA); }

DEFINE_INTRINSIC_out_Float_in_Float_Float(atan2,        ([](float y, float x){ return atan2(y, x); }) )

#undef DEFINE_INTRINSIC_out_Float_in_Float_Float
//=============================================================================
struct IntrinsicTrait
{
    char const* name;
    IntrinsicFct fct;
};
IntrinsicTrait intrinsicTraits[] = {
    { "acos",           Intrinsic_acos },
    { "asin",           Intrinsic_asin },
    { "atan",           Intrinsic_atan },
    { "atan2",          Intrinsic_atan2 },
    { "cos",            Intrinsic_cos },
    { "exp",            Intrinsic_exp },
    { "is_integer",     Intrinsic_IsInteger },
    { "is_numeric",     Intrinsic_IsNumeric },
    { "is_same_type",   Intrinsic_IsSameType },
    { "isfinite",       Intrinsic_isfinite },
    { "isinf",          Intrinsic_isinf },
    { "isnan",          Intrinsic_isnan },
    { "list_length",    Intrinsic_ListLength },
    { "log",            Intrinsic_log },
    { "print",          Intrinsic_Print },
    { "sin",            Intrinsic_sin },
    { "sqrt",           Intrinsic_sqrt },
    { "tan",            Intrinsic_tan },
    { "type_name_of",   Intrinsic_TypeNameOf },
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

