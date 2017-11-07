#ifndef ToolsUI_Label_H
#define ToolsUI_Label_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include <UserInterface/Component.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/Movable.h>
#include <UserInterface/Text.h>

namespace sg {
namespace toolsui {
//=============================================================================
class Label : public ui::Component
            , public ui::IMovable
{
    typedef ui::Component parent_type;
public:
    Label(std::wstring const& iText, ui::Length const& iLineWidth = ui::Relative(1.f));
    ~Label();
    void SetText(std::wstring const& iText);
    void SetLineWidth(ui::Length const& iLineWidth);
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
private:
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualUpdatePlacement() override;
    virtual ui::Component* VirtualAsComponent() override { return this; }
private:
    ui::FrameProperty m_frameProperty;
    ui::ParagraphStyle m_paragraphStyle;
    ui::Text m_text;
    float2 m_textPos;
    ui::Length m_lineWidth;
};
//=============================================================================
}
}

#endif
