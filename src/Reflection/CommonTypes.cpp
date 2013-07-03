#include "stdafx.h"

#include "CommonTypes.h"

#include <Math/Vector.h>

namespace sg {
namespace reflection {
    REFLECTION_TYPE_WRAPPER_BEGIN(float2)
        REFLECTION_TYPE_DOC("2D vector")
        REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y coordinate")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(float3)
        REFLECTION_TYPE_DOC("3D vector")
        REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[2], z, "z coordinate")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(float4)
        REFLECTION_TYPE_DOC("4D vector")
        REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[2], z, "z coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[3], w, "w coordinate")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(i32vec2)
        REFLECTION_TYPE_DOC("2D vector (32bits integer)")
        REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y coordinate")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(i32vec3)
        REFLECTION_TYPE_DOC("3D vector (32bits integer)")
        REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[2], z, "z coordinate")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(i32vec4)
        REFLECTION_TYPE_DOC("4D vector (32bits integer)")
        REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[2], z, "z coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[3], w, "w coordinate")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(u32vec2)
        REFLECTION_TYPE_DOC("2D vector (32bits unsigned integer)")
        REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y coordinate")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(u32vec3)
        REFLECTION_TYPE_DOC("3D vector (32bits unsigned integer)")
        REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[2], z, "z coordinate")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(u32vec4)
        REFLECTION_TYPE_DOC("4D vector (32bits unsigned integer)")
        REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[2], z, "z coordinate")
        REFLECTION_NAMED_PROPERTY_DOC(_[3], w, "w coordinate")
    REFLECTION_TYPE_WRAPPER_END

    REFLECTION_TYPE_WRAPPER_BEGIN(Color3f)
        REFLECTION_TYPE_DOC("Color RGB as floats")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[0], r, "r component")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[1], g, "g component")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[2], b, "b component")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(Color4f)
        REFLECTION_TYPE_DOC("Color RGBA as floats")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[0], r, "r component")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[1], g, "g component")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[2], b, "b component")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[3], a, "a component")
    REFLECTION_TYPE_WRAPPER_END
    REFLECTION_TYPE_WRAPPER_BEGIN(R8G8B8A8)
        REFLECTION_TYPE_DOC("Color RGBA, 8 bits per component")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[0], r, "r component")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[1], g, "g component")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[2], b, "b component")
        REFLECTION_NAMED_PROPERTY_DOC(m_data._[3], a, "a component")
    REFLECTION_TYPE_WRAPPER_END
}
}
