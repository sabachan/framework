#ifndef UserInterface_ScrollingContainer_H
#define UserInterface_ScrollingContainer_H

#include "ClippingContainer.h"
#include "Container.h"
#include "FitMode.h"
#include "Length.h"
#include "Magnifier.h"
#include <Math/Box.h>


namespace sg {
namespace ui {
//=============================================================================
class ClippingContainer;
class IMovable;
class ScrollingContainer_SubContainer;
class TextureDrawer;
//=============================================================================
class ScrollingContainer : public Container
                         , public IMagnifiable

{
    typedef Container parent_type;
public:
    struct Properties
    {
        FitMode2 fitMode;
        Length scrollingSpeed;
        LengthBox2 scrollingMargins;
    };
public:
    virtual ~ScrollingContainer() override;
    void SetContent(IMovable* iContent);
    void ScrollInUnitIFP(float2 const& iOffset);
    void ScrollToBoxInContentIFP(box2f const& iBoxInContent);
    float2 GetRelativeScrollPosition() const { return m_relativeScrollPosition; }
    box2f GetRelativeWindowOnContent() const;
    void GetWindowAndContentBox(box2f& oWindowBox, box2f& oContentBox) const;
    Properties const& GetProperties() const { return m_properties; }
protected:
    ScrollingContainer(Magnifier const& iMagnifier, TextureDrawer const* iTextureDrawer, Properties const& iProperties);
    virtual void VirtualSetVisibleBarsAndGetBarsMargins(bool2 iVisibleBars, box2f& oBarsMargins);
    void SetBarsMargins(box2f const& iBarsMargins);
    box2f const& BarsMargins() const { return m_barsMargins; }
    bool2 VisibleBars() const { return m_visibleBars; }
    virtual void VirtualOnChildInvalidatePlacement() override { InvalidatePlacement(); }
private:
    virtual void VirtualUpdatePlacement() override;
    box2f GetContentMargins() const;
#if SG_ENABLE_ASSERT
    void CheckConstraintsOnItem(IMovable* iContent);
#endif
private:
    refptr<ScrollingContainer_SubContainer> m_subContainer;
    refptr<ClippingContainer> m_clippingContainer;
    safeptr<IMovable> m_content;
    Properties m_properties;
    float2 m_relativeScrollPosition;
    box2f m_barsMargins;
    bool2 m_visibleBars;
};
//=============================================================================
}
}

#endif
