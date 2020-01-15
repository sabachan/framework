#ifndef Include_DeclareMetaclasses
#error "this file should be included only by one .cpp"
#endif

// This file is used to force initialization of static objects used to
// register metaclasses. It should be included in a used compilation unit,
// eg the .cpp file where main function is defined.

#include <Reflection/BaseClass.h>

REFLECTION_DECLARE_METACLASS((sg,ui), CirclePathDrawerDescriptor)
REFLECTION_DECLARE_METACLASS((sg,ui), GenericStyleGuide)
REFLECTION_DECLARE_METACLASS((sg,ui), Length)
REFLECTION_DECLARE_METACLASS((sg,ui), Length2)
REFLECTION_DECLARE_METACLASS((sg,ui), OffscreenCommon)
REFLECTION_DECLARE_METACLASS((sg,ui), ParagraphStyle)
REFLECTION_DECLARE_METACLASS((sg,ui), TextFormatScript)
REFLECTION_DECLARE_METACLASS((sg,ui), TextStyle)
REFLECTION_DECLARE_METACLASS((sg,ui), TextureDrawerDescriptor)
REFLECTION_DECLARE_METACLASS((sg,ui), TFSInstruction)
REFLECTION_DECLARE_METACLASS((sg,ui), TFSInsert)
REFLECTION_DECLARE_METACLASS((sg,ui), TFSSkip)
REFLECTION_DECLARE_METACLASS((sg,ui), TFSStyle)
REFLECTION_DECLARE_METACLASS((sg,ui), UniformDrawerDescriptor)
