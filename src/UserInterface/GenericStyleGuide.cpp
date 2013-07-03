#include "stdafx.h"

#include "GenericStyleGuide.h"

#include <Reflection/CommonTypes.h>
#include <Rendering/ColorSpace.h>
#include <Rendering/ColorUtils.h>
#include <Rendering/Material.h>
#include <Rendering/RenderDevice.h>
#include <UserInterface/TypefaceFromBitMapFont.h>

namespace sg {
namespace ui {
//=============================================================================
GenericStyleGuide::GenericStyleGuide()
{
    rendering::RenderDevice* renderDevice = rendering::RenderDevice::GetIFP();
    SG_ASSERT(nullptr != renderDevice);
    m_typefaces["Default"] = new ui::TypefaceFromBitMapFont(renderDevice);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
GenericStyleGuide::~GenericStyleGuide()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GenericStyleGuide::OnCreatedOrModified()
{
    for(auto& it : m_uniformDrawers)
        it.second->GetDrawer();
    for(auto& it : m_textureDrawers)
        it.second->GetDrawer();
    for(auto& it : m_circlePathDrawers)
        it.second->GetDrawer();

    m_genLinearColors.clear();
    for(auto& it : m_colors)
    {
        Color4f const lRGBa = srgba_to_lRGBa(it.second);
        auto r = m_genLinearColors.emplace(it.first, lRGBa);
        SG_ASSERT(r.second);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, ui), GenericStyleGuide)
    REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(colors, "")
    REFLECTION_m_PROPERTY_DOC(lengths, "")
    REFLECTION_m_PROPERTY_DOC(vectors, "")
    REFLECTION_m_PROPERTY_DOC(uniformDrawers, "")
    REFLECTION_m_PROPERTY_DOC(textureDrawers, "")
    REFLECTION_m_PROPERTY_DOC(circlePathDrawers, "")
    REFLECTION_m_PROPERTY_DOC(tfs, "")
    REFLECTION_m_PROPERTY_DOC(textStyles, "")
    REFLECTION_m_PROPERTY_DOC(paragraphStyles, "")
REFLECTION_CLASS_END
//=============================================================================
}
}
