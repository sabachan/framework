#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "ScrollingContainer.h"

#include "Common.h"
#include <UserInterface/FitMode.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/Movable.h>
#include <UserInterface/PointerEvent.h>
#include <UserInterface/SensitiveArea.h>
#include <System/MouseUtils.h>
#include <System/UserInputEvent.h>

namespace sg {
namespace toolsui {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE ScrollingContainer::Properties MakeProperties(ui::FitMode2 iFitMode, ui::Length iScrollingSpeed)
{
    ScrollingContainer::Properties prop;
    prop.fitMode = iFitMode;
    prop.scrollingSpeed = iScrollingSpeed;
    return prop;
}
SG_FORCE_INLINE ui::TextureDrawer const* GetCommonTextureDrawer()
{
    toolsui::Common const& common = toolsui::Common::Get();
    ui::GenericStyleGuide const* styleGuide = common.StyleGuide();
    ui::TextureDrawer const* textureDrawer = styleGuide->GetTextureDrawer(common.Default);
    return textureDrawer;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_FORCE_INLINE ui::Magnifier const& GetCommonMagnifier()
{
    toolsui::Common const& common = toolsui::Common::Get();
    return common.GetMagnifier();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ScrollingContainer::ScrollingContainer(Properties const& iProperties)
    : ui::ScrollingContainer(GetCommonMagnifier(), GetCommonTextureDrawer(), iProperties)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ScrollingContainer::ScrollingContainer()
    : ScrollingContainer(ui::FitMode2(ui::FitMode::FitToFrameOnly))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ScrollingContainer::ScrollingContainer(ui::FitMode2 const& iFitMode)
    : ScrollingContainer(MakeProperties(iFitMode, ui::Magnifiable(30)))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::VirtualSetVisibleBarsAndGetBarsMargins(bool2 iVisibleBars, box2f& oBarsMargins)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    float const thickness = styleGuide.GetLength(common.MinManipulationThickness).Resolve(magnification, std::numeric_limits<float>::lowest());

    oBarsMargins = box2f::FromMinMax(float2(0,0), float2(iVisibleBars.y() ? -thickness : 0.f, iVisibleBars.x() ? -thickness : 0.f));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::VirtualOnDraw(ui::DrawContext const& iContext)
{
    parent_type::VirtualOnDraw(iContext);

    bool2 const visibleBars = VisibleBars();
    if(bool2(false) != visibleBars)
    {
        box2f const relativeBox = GetRelativeWindowOnContent();
        Common const& common = Common::Get();
        float const magnification = common.GetMagnifier().Magnification();
        ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
        ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);

        box2f const& placementBox = PlacementBox_AssumeUpToDate();
        box2f const& barsMargins = BarsMargins();
        float2 const lineMargin = styleGuide.GetVector(common.LineMargin2).Resolve(magnification, float2(std::numeric_limits<float>::lowest()));
        float const minBarLength = styleGuide.GetLength(common.MinManipulationLength).Resolve(magnification, std::numeric_limits<float>::lowest());
        Color4f const color = styleGuide.GetLinearColor(common.FillColorB);
        if(visibleBars.x())
        {
            SG_ASSERT_NOT_IMPLEMENTED();
        }
        if(visibleBars.y())
        {
            box2f const fullBarBox = box2f::FromMinMax(
                placementBox.Corner(BitSet<2>(0x1)) + float2(barsMargins.Max().x(), lineMargin.y()),
                placementBox.Max() + float2(-lineMargin.x(), barsMargins.Max().y() - lineMargin.y()));
            float const fullBarLength = fullBarBox.Delta().y();
            box1f barBoxY = relativeBox.SubBox<1>(1) * fullBarLength + fullBarBox.Min().y();
            if(minBarLength > barBoxY.Delta())
            {
                float const barLength = std::min(minBarLength, 0.3f * fullBarLength);
                float const relativePosition = GetRelativeScrollPosition().y();
                barBoxY = box1f::FromMinDelta((fullBarLength-barLength) * relativePosition + fullBarBox.Min().y(), barLength);
            }
            box2f barBox = fullBarBox.SubBox<1>(0) * barBoxY;
            if(barBox.NVolume_NegativeIfNonConvex() > 0)
                drawer->DrawQuad(iContext, barBox, color);
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent)
{
    // TODO: bar manipulation
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
}
//=============================================================================
}
}
#endif
