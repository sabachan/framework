#include "stdafx.h"
#include "InitShutdown.h"

#include "Metaclass.h"
#include "ObjectDatabase.h"

namespace sg {
namespace reflection {
//=============================================================================
void Init()
{
    IdentifierSymbol::Init();
    InitMetaclasses();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Shutdown()
{
    ShutdownMetaclasses();
    IdentifierSymbol::Shutdown();
}
//=============================================================================
}
}
