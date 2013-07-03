#include "stdafx.h"

#include "Common.h"

#include <Core/Config.h>
#include <UserInterface/GenericStyleGuide.h>

#if SG_ENABLE_TOOLS
#include "Toolbox.h"
#include <Core/Assert.h>
#include <Core/Log.h>
#include <Core/Tool.h>
#include <Rendering/ColorSpace.h>
#include <Reflection/ObjectDatabase.h>
#include <ObjectScript/Reader.h>
#include <UserInterface/Container.h>
#include <UserInterface/PointerEvent.h>
#endif

namespace sg {
namespace toolsui {
//=============================================================================
#if SG_ENABLE_TOOLS
namespace {
class ContainerForModal : public ui::Container
{
    typedef ui::Container parent_type;
protected:
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override
    {
        // TODO: This is not the correct place where to update the time server.
        // It should be done at the beginning of the frame, but there is no
        // mechanism to do that yet.
        CommonTimeServer::Get().Update();

        if(!this->Empty())
        {
            Common const& common = Common::Get();
            ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
            float const magnification = common.GetMagnifier().Magnification();
            box2f const& placementBox = PlacementBox();
            box2f const box = placementBox;
            ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
            Color4f const color = Color4f(0,0,0,0.7f);
            drawer->DrawQuad(iContext, box, color);
        }
        parent_type::VirtualOnDraw(iContext);
    }
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override
    {
        parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
        if(!this->Empty())
        {
            if(iPointerEvent.Event().IsPreMasked()) iPointerEvent.Event().SetMaskedFromPremasked();
            else if(!iPointerEvent.Event().IsMasked()) iPointerEvent.Event().SetMasked();
        }
    }
};
}
#endif
//=============================================================================
Common::Common()
    : m_styleGuide()
#if SG_ENABLE_TOOLS
    , m_windowContainer()
    , m_modalContainer()
    , m_toolbox()
    , m_magnifier()
    , m_timeServer()
#endif
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Common::~Common()
{
#if SG_ENABLE_TOOLS
    m_modalContainer = nullptr;
    m_windowContainer = nullptr;
    m_toolbox = nullptr;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_TOOLS
void Common::VirtualOnCreated(reflection::ObjectCreationContext& iContext)
{
    reflection_parent_type::VirtualOnCreated(iContext);
    m_styleGuide->EndCreationIFN(iContext);
    m_modalContainer = new ContainerForModal;
    m_windowContainer = new ui::Container;
    m_toolbox = new Toolbox;
    m_windowContainer->RequestAddToFront(m_toolbox.get(), 1);
    ResolveStyleGuide();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Common::ResolveStyleGuide()
{
    m_resolvedStyleGuide.BGColor = m_styleGuide->GetLinearColor(FillColorA);
    m_resolvedStyleGuide.ButtonFillColor = m_styleGuide->GetLinearColor(FillColorB);
    m_resolvedStyleGuide.ButtonHighlightColor = m_styleGuide->GetLinearColor(FillColorB1);
    m_resolvedStyleGuide.ButtonLineColor = m_styleGuide->GetLinearColor(LineColorB);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg,toolsui), Common)
    REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(styleGuide, "")
REFLECTION_CLASS_END
//=============================================================================
#if SG_ENABLE_TOOLS
void GetButtonLikeRenderParam(ButtonLikeRenderParam& oParam, box2f const& iPlacementBox, bool iIsActivated, bool iIsHover, bool iIsClicked)
{
    Common const& common = Common::Get();
    Common::ResolvedStyleGuide const& resolvedStyleGuide = common.GetResolvedStyleGuide();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& box = iPlacementBox;
    oParam.outBox = box;

    bool const prototypeMode = GetEditableValue("Tools UI/Button/Prototype mode", "Enable manipulating different values", false);
    float lineThickness = styleGuide.GetLength(iIsClicked ? common.LineThickness1 : common.LineThickness0).Resolve(magnification, std::numeric_limits<float>::lowest());
    if(prototypeMode)
    {
        float const borderThickness          = 1.f * GetEditableValue("Tools UI/Button/Border thickness", "Border thickness", int(0), 0, 8, 1);
        float const highlightBorderThickness = 1.f * GetEditableValue("Tools UI/Button/Highlight border thickness", "Border thickness when highlighted", int(1), 0, 8, 1);
        float const clickedBorderThickness   = 1.f * GetEditableValue("Tools UI/Button/Clicked border thickness", "Border thickness when clicked", int(1), 0, 8, 1);

        lineThickness = iIsClicked ? clickedBorderThickness : iIsHover ? highlightBorderThickness : borderThickness;
    }

    oParam.inBox = box2f::FromMinMax(box.Min() + lineThickness, box.Max() - lineThickness);
    oParam.highlightColor = resolvedStyleGuide.ButtonHighlightColor;
    oParam.baseColor = resolvedStyleGuide.ButtonFillColor;
    oParam.fillColor = (iIsClicked || iIsHover) ? oParam.highlightColor : oParam.baseColor;
    oParam.lineColor = resolvedStyleGuide.ButtonLineColor;

    auto ToUnactivated = [](Color4f const& c)
    {
        auto const hsv = ConvertTo<ColorSpace::HueSaturationValue>(ColorSpacePoint<ColorSpace::LinearRGB>(float4(c).xyz()));
        //hsv.y() *= 0.9f;
        //hsv.z() *= 0.7f;
        auto const d = ConvertTo<ColorSpace::LinearRGB>(hsv);
        float4 const d4f = float3(d).Append(c.a());
        return Color4f(d4f);
    };

    if(!iIsActivated)
    {
        oParam.fillColor = ToUnactivated(oParam.fillColor);
        oParam.lineColor = ToUnactivated(oParam.lineColor);
    }
}
#endif
//=============================================================================
}
}
