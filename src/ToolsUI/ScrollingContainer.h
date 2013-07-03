#ifndef ToolsUI_ScrollingContainer_H
#define ToolsUI_ScrollingContainer_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include <Math/Box.h>
#include <UserInterface/ClippingContainer.h>
#include <UserInterface/Container.h>
#include <UserInterface/FitMode.h>
#include <UserInterface/Length.h>
#include <UserInterface/ScrollingContainer.h>
#include <UserInterface/SensitiveArea.h>
#include <UserInterface/WheelHelper.h>

namespace sg {
namespace ui {
class IMovable;
}
}

namespace sg {
namespace toolsui {
//=============================================================================
class ScrollingContainer : public ui::ScrollingContainer
{
    typedef ui::ScrollingContainer parent_type;
public:
    using ui::ScrollingContainer::Properties;
    ScrollingContainer();
    explicit ScrollingContainer(ui::FitMode2 const& iFitMode);
    explicit ScrollingContainer(Properties const& iProperties);
    virtual void VirtualSetVisibleBarsAndGetBarsMargins(bool2 iVisibleBars, box2f& oBarsMargins) override;
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override;
};
//=============================================================================
}
}

#endif
