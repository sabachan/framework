#ifndef ToolsUI_Common_H
#define ToolsUI_Common_H

#include <Core/Config.h>
#include <Reflection/BaseClass.h>

#if SG_ENABLE_TOOLS
#include <Core/FastSymbol.h>
#include <Core/Singleton.h>
#include <Core/TimeFromWallClock.h>
#include <Rendering/Color.h>
#include <Rendering/ColorSpace.h>
#include <Math/Box.h>
#include <UserInterface/Magnifier.h>
#endif

namespace sg {
class TimeServer;
namespace ui {
class Container;
class GenericStyleGuide;
}
}

namespace sg {
namespace toolsui {
//=============================================================================
#if SG_ENABLE_TOOLS
class CommonTimeServer : public TimeFromWallClock
                       , public Singleton<CommonTimeServer>
{
    PARENT_SAFE_COUNTABLE(TimeFromWallClock)
public:
    CommonTimeServer() : TimeFromWallClock() {}
};
#endif
//=============================================================================
#if SG_ENABLE_TOOLS
class Toolbox;
#endif
//=============================================================================
// Note: As there are instances of Common type in object scripts, we need to
// declare this class also in final version.
class Common : public reflection::BaseClass
#if SG_ENABLE_TOOLS
             , public Singleton<Common>
#endif
{
    PARENT_SAFE_COUNTABLE(reflection::BaseClass)
    REFLECTION_CLASS_HEADER(Common, reflection::BaseClass)
private:
    Common();
public:
    ~Common();
#if SG_ENABLE_TOOLS
    struct ResolvedStyleGuide
    {
        // these color are in linear RGB
        Color4f BGColor;
        Color4f ButtonFillColor;
        Color4f ButtonHighlightColor;
        Color4f ButtonLineColor;
    };
    ResolvedStyleGuide const& GetResolvedStyleGuide() const {  return m_resolvedStyleGuide; }
    ui::GenericStyleGuide const* StyleGuide() const { return m_styleGuide.get(); }
    ui::Container* WindowContainer() const { return m_windowContainer.get(); }
    ui::Container* ModalContainer() const { return m_modalContainer.get(); }
    ui::Magnifier const& GetMagnifier() const { return m_magnifier; }
    ui::Magnifier& GetMagnifier() { return m_magnifier; }
    //struct ShutdowwnEvent : public Observable<ShutdowwnEvent> {};
private:
    virtual void VirtualOnCreated(reflection::ObjectCreationContext& iContext) override;
    void ResolveStyleGuide();
public:
#define DECLARE_FAST_SYMBOL(NAME) FastSymbol const NAME { #NAME };
    DECLARE_FAST_SYMBOL(Default)
    // color
    DECLARE_FAST_SYMBOL(FillColorA)
    DECLARE_FAST_SYMBOL(FillColorA1)
    DECLARE_FAST_SYMBOL(FillColorA2)
    DECLARE_FAST_SYMBOL(FillColorB)
    DECLARE_FAST_SYMBOL(FillColorB1)
    DECLARE_FAST_SYMBOL(FillColorC)
    DECLARE_FAST_SYMBOL(LineColorA)
    DECLARE_FAST_SYMBOL(LineColorA1)
    DECLARE_FAST_SYMBOL(LineColorB)
    // length
    DECLARE_FAST_SYMBOL(LineThickness0)
    DECLARE_FAST_SYMBOL(LineThickness1)
    DECLARE_FAST_SYMBOL(WindowLineThickness)
    DECLARE_FAST_SYMBOL(MinManipulationThickness)
    DECLARE_FAST_SYMBOL(MinManipulationLength)
    // length2
    DECLARE_FAST_SYMBOL(LineMargin0)
    DECLARE_FAST_SYMBOL(LineMargin1)
    DECLARE_FAST_SYMBOL(LineMargin2)
    DECLARE_FAST_SYMBOL(TextMargin)
    DECLARE_FAST_SYMBOL(WindowButtonSize)
    // text styles
    DECLARE_FAST_SYMBOL(Symbols)
#undef DECLARE_FAST_SYMBOL
#endif
private:
    refptr<ui::GenericStyleGuide const> m_styleGuide;
#if SG_ENABLE_TOOLS
    refptr<ui::Container> m_windowContainer;
    refptr<ui::Container> m_modalContainer;
    refptr<Toolbox> m_toolbox;
    ui::Magnifier m_magnifier;
    CommonTimeServer m_timeServer;
    ResolvedStyleGuide m_resolvedStyleGuide;
#endif
};
//=============================================================================
#if SG_ENABLE_TOOLS
struct ButtonLikeRenderParam
{
    box2f outBox;
    box2f inBox;
    Color4f lineColor;
    Color4f fillColor;
    Color4f highlightColor;
    Color4f baseColor;
};
void GetButtonLikeRenderParam(ButtonLikeRenderParam& oParam, box2f const& iPlacementBox, bool iIsActivated, bool iIsHover, bool iIsClicked);
#endif
//=============================================================================
}
}

#endif
