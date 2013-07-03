#include "stdafx.h"

#include "Length.h"

namespace sg {
namespace ui {
//=============================================================================
REFLECTION_TYPE_BEGIN_WITH_CONSTRUCTS((sg, ui), Length, true, true)
REFLECTION_TYPE_DOC("A length is computed as the sum of 3 components: unit, magnifiable and relative.")
REFLECTION_PROPERTY_DOC(unit, "Part of length that is constant.")
REFLECTION_PROPERTY_DOC(magnifiable, "Part of length that is multiplied by magnification.")
REFLECTION_PROPERTY_DOC(relative, "Part of length that is relative to parent length.")
REFLECTION_TYPE_END
//=============================================================================
REFLECTION_TYPE_BEGIN_WITH_CONSTRUCTS((sg, ui), Length2, true, true)
REFLECTION_TYPE_DOC("2D vector expressed as the sum of 3 components: unit, magnifiable and relative.")
REFLECTION_NAMED_PROPERTY_DOC(_[0], x, "x component")
REFLECTION_NAMED_PROPERTY_DOC(_[1], y, "y component")
REFLECTION_TYPE_END
//=============================================================================
}
}

