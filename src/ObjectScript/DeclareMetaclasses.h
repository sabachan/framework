#ifndef Include_DeclareMetaclasses
#error "this file should be included only by one .cpp"
#endif

// This file is used to force initialization of static objects used to
// register metaclasses. It should be included in a used compilation unit,
// eg the .cpp file where main function is defined.

#include <Reflection/BaseClass.h>

REFLECTION_DECLARE_METACLASS((sg,objectscript), Alias)
REFLECTION_DECLARE_METACLASS((sg,objectscript), Callable)
REFLECTION_DECLARE_METACLASS((sg,objectscript), FreeVariable)
REFLECTION_DECLARE_METACLASS((sg,objectscript), Function)
REFLECTION_DECLARE_METACLASS((sg,objectscript), IntrinsicCall)
REFLECTION_DECLARE_METACLASS((sg,objectscript), Template)
REFLECTION_DECLARE_METACLASS((sg,objectscript), TemplateAlias)
REFLECTION_DECLARE_METACLASS((sg,objectscript), TemplateNamespace)
REFLECTION_DECLARE_METACLASS((sg,objectscript), Variable)

