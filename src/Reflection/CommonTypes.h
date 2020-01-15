#ifndef Reflection_CommonTypes_H
#define Reflection_CommonTypes_H

#include "BaseClass.h"
#include <Math/Box.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>
#include <Rendering/Color.h>

namespace sg {
namespace reflection {
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Vector<float, 2>), float2)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Vector<float, 3>), float3)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Vector<float, 4>), float4)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Vector<i32, 2>), i32vec2)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Vector<i32, 3>), i32vec3)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Vector<i32, 4>), i32vec4)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Vector<u32, 2>), u32vec2)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Vector<u32, 3>), u32vec3)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Vector<u32, 4>), u32vec4)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (TemplateQuaternion<float>), quaternion)

    REFLECTION_TYPE_WRAPPER_HEADER((sg, math), (Box<i32, 2>), box2i)


    REFLECTION_TYPE_WRAPPER_HEADER((sg, rendering), (Color<float, 3>), Color3f)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, rendering), (Color<float, 4>), Color4f)
    REFLECTION_TYPE_WRAPPER_HEADER((sg, rendering), (Color<u8, 4>), R8G8B8A8)
    // TODO: matrices, ...
}
}

#endif
