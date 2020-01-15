#ifndef Tools_CommonTools_Toolbox_H
#define Tools_CommonTools_Toolbox_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS && !defined(SG_TOOLBOX_CPP)
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include <Reflection/BaseClass.h>
#include <ToolsUI/Common.h>
#if SG_ENABLE_TOOLS
#include <ToolsUI/Window.h>
#include <UserInterface/Component.h>
#endif

namespace sg {
namespace toolsui {
    class TreeView;
}
}

namespace sg {
namespace commontools {
//=============================================================================
#if SG_ENABLE_TOOLS
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
    toolsui::WindowHandler m_windowHandler;
    refptr<toolsui::TreeView> m_tree;
    size_t m_toolboxModificationStamp;
};
#endif
//=============================================================================
class ToolboxLoader : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(ToolboxLoader, reflection::BaseClass)
private:
#if SG_ENABLE_TOOLS
    virtual void VirtualOnCreated(reflection::ObjectCreationContext& iContext) override;
#endif
private:
    refptr<toolsui::Common> m_common;
#if SG_ENABLE_TOOLS
    refptr<Toolbox> m_toolbox;
#endif
};
//=============================================================================
}
}

#endif
