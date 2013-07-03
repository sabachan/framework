#ifndef Include_DeclareMetaclasses
#error "this file should be included only by one .cpp"
#endif

// This file is used to force initialization of static objects used to
// register metaclasses. It should be included in a used compilation unit,
// eg the .cpp file where main function is defined.

#include <Reflection/BaseClass.h>
#include <Core/Config.h>

#if SG_ENABLE_TOOLS
REFLECTION_DECLARE_METACLASS((sg,image,tool), FontCodeGenerator)
#endif

