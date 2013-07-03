#ifndef Include_DeclareMetaclasses
#error "this file should be included only by one .cpp"
#endif

// This file is used to force initialization of static objects used to
// register metaclasses. It should be included in a used compilation unit,
// eg the .cpp file where main function is defined.

#include <Core/Config.h>
#include <Reflection/BaseClass.h>

REFLECTION_DECLARE_METACLASS((sg,reflection), BaseClass)
REFLECTION_DECLARE_METACLASS((sg,reflection), BaseType)
#if SG_ENABLE_UNIT_TESTS
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestClass_A)
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestClass_B)
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestClass_C)
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestClass_D)
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestStruct_A)
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestStruct_B)
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestTemplate_A_float)
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestTemplate_A_int)
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestTemplate_B_int_int)
REFLECTION_DECLARE_METACLASS((sg,reflectionTest), TestTemplate_C_int_int_int)
#endif

