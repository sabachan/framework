#include "stdafx.h"

#include <Reflection/CommonTypes.h>
#include "TextStyle.h"

namespace sg {
namespace ui {
//=============================================================================
REFLECTION_TYPE_BEGIN_WITH_CONSTRUCTS((sg,ui), TextStyle, false, true)
REFLECTION_TYPE_DOC("Text style")
REFLECTION_PROPERTY_DOC(fillColor, "")
REFLECTION_PROPERTY_DOC(strokeColor, "")
REFLECTION_PROPERTY_DOC(strokeSize, "")
REFLECTION_PROPERTY_DOC(size, "")
REFLECTION_PROPERTY_DOC(fontFamilyName, "")
REFLECTION_PROPERTY_DOC(bold, "")
REFLECTION_PROPERTY_DOC(italic, "")
REFLECTION_PROPERTY_DOC(superscript, "")
REFLECTION_PROPERTY_DOC(subscript, "")
REFLECTION_PROPERTY_DOC(underlined, "")
REFLECTION_PROPERTY_DOC(strikeThrough, "")
REFLECTION_TYPE_END
//=============================================================================
REFLECTION_TYPE_BEGIN_WITH_CONSTRUCTS((sg,ui), ParagraphStyle, false, true)
REFLECTION_TYPE_DOC("Paragraph style")
REFLECTION_PROPERTY_DOC(alignment, "")
REFLECTION_PROPERTY_DOC(justified, "")
REFLECTION_PROPERTY_DOC(lineWidth, "a negative line width means that text shouldn't be wraped");
REFLECTION_PROPERTY_DOC(lineGap, "")
REFLECTION_PROPERTY_DOC(balanced, "")
REFLECTION_TYPE_END
//=============================================================================
}
}
