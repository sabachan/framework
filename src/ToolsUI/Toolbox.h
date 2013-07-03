#ifndef ToolsUI_Toolbox_H
#define ToolsUI_Toolbox_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include "Window.h"
#include <UserInterface/Component.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/Movable.h>
#include <UserInterface/Text.h>

namespace sg {
namespace toolsui {
//=============================================================================
class Toolbox : public ui::Container
{
    typedef ui::Container parent_type;
public:
    Toolbox();
    ~Toolbox();
    void ShowToolboxWindow(bool iShow);
protected:
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override;
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualOnInsertInUI() override;
    virtual void VirtualOnRemoveFromUI() override;
private:
    void CreateToolboxWindow();
    void UpdateToolboxWindowIFN();
    void UpdateToolboxWindow();
private:
    refptr<ui::Component> m_mainButton;
    WindowHandler m_windowHandler;
    refptr<ui::VerticalListLayout> m_list;
    size_t m_toolboxModificationStamp;
};
//=============================================================================
}
}

#endif
