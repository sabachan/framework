#ifndef Include_DeclareMetaclasses
#error "this file should be included only by one .cpp"
#endif

// This file is used to force initialization of static objects used to
// register metaclasses. It should be included in a used compilation unit,
// eg the .cpp file where main function is defined.

#include <Reflection/BaseClass.h>

REFLECTION_DECLARE_METACLASS((sg,renderengine), AbstractCompositingInstructionDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), AbstractConstantDatabaseDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), AbstractResolutionDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), AbstractSurfaceDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), ClearSurfacesDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), CompositingDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), CompositingLayerDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), ConstantDatabaseDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), GenerateMipmapDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), InputConstantDatabaseDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), InputSurfaceDescriptor)
//REFLECTION_DECLARE_METACLASS((sg,renderengine), MaterialDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), OutputSurfaceDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), ResolutionDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), ResolutionFromInputSurfaceDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), ResolutionFromOutputSurfaceDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), ResolutionFromTextureSurfaceDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), SamplerDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), SetRenderTargetsDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), ShaderPassDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), ShaderResourceDatabaseDescriptor)
REFLECTION_DECLARE_METACLASS((sg,renderengine), SurfaceDescriptor)

