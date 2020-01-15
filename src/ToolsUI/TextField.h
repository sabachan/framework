#ifndef ToolsUI_TextField_H
#define ToolsUI_TextField_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include "Common.h"
#include <UserInterface/AnimFactor.h>
#include <UserInterface/Component.h>
#include <UserInterface/EditableText.h>
#include <UserInterface/Focusable.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/Movable.h>
#include <UserInterface/SensitiveArea.h>
#include <UserInterface/Text.h>

namespace sg {
namespace toolsui {
//=============================================================================
class TextField : public ui::EditableText
{
    typedef ui::EditableText parent_type;
    typedef ui::EditableText focusable_parent_type;
public:
    TextField(std::wstring const& iText, ui::Length const& iLineWidth = ui::Relative(1.f));
    ~TextField();
    void SetLineWidth(ui::Length const& iLineWidth);
protected:
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualOnRefreshCursor() override;
private:
    ui::AnimFactorOnTimeServer<toolsui::CommonTimeServer> m_cursorVisibilityStabilizer;
    ui::CyclicAnimFactorOnTimeServer<toolsui::CommonTimeServer> m_cursorBlinking;
};
//=============================================================================
}
}

#endif
