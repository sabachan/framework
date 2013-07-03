#ifndef ObjectScript_Writer_H
#define ObjectScript_Writer_H

#if !SG_ENABLE_TOOLS
#error "Writer.h shouldn't be included without SG_ENABLE_TOOLS"
#endif

#include <Core/SmartPtr.h>
#include <string>
#include <unordered_map>

namespace sg {
namespace reflection {
class BaseClass;
class IdentifierNode;
class IPrimitiveData;
class ObjectDatabase;
}
}

namespace sg {
namespace objectscript {
//=============================================================================
void WriteObjectScript(std::string& oFileContent, reflection::ObjectDatabase const& iObjectDatabase);
//=============================================================================
}
}

#endif
