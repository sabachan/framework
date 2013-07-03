#ifndef ObjectScript_SimpleReader_H
#define ObjectScript_SimpleReader_H

#include <Core/Config.h>
#include <Core/FilePath.h>

namespace sg {
namespace reflection {
class ObjectDatabase;
}
}

namespace sg {
namespace simpleobjectscript {
//=============================================================================
bool ReadObjectScriptROK(FilePath const& iFile, reflection::ObjectDatabase& ioObjectDatabase /*, errorhandler ?*/);
bool ReadObjectScriptROK(char const* iFileContent, reflection::ObjectDatabase& ioObjectDatabase /*, errorhandler ?*/);
//=============================================================================
}
}

#endif
