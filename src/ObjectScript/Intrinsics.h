#ifndef ObjectScript_Intrinsics_H
#define ObjectScript_Intrinsics_H

#include <Reflection/PrimitiveData.h>

namespace sg {
namespace objectscript {
//=============================================================================
class IErrorHandler;
//=============================================================================
typedef bool (*IntrinsicFct)(refptr<reflection::IPrimitiveData>& oResult, reflection::PrimitiveDataList const& iArgs, std::string& oErrorMessage);
//=============================================================================
IntrinsicFct GetIntrinsic(char const* iName);
//=============================================================================
}
}

#endif


