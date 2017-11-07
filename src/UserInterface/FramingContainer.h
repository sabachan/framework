#ifndef UserInterface_FramingContainer_H
#define UserInterface_FramingContainer_H

#include <Math/Box.h>
#include "Container.h"
#include "FitMode.h"
#include "FrameProperty.h"
#include "Movable.h"

namespace sg {
namespace ui {
//=============================================================================
class Magnifier;
//=============================================================================
// A FramingContainer is useful to add borders around a component that computes
// its size from its parent. It can also be used to add a movable capacity to
// its content.
class FramingContainer : public Container
                       , public IMovable
{
    typedef ui::Container parent_type;
public:
    FramingContainer(Magnifier const& iMagnifier, FrameProperty const& iFrameProperty, FitMode2 iFitMode);
    virtual ~FramingContainer() override;
    void SetContent(Component* iContent);
    void SetFrameProperty(ui::FrameProperty const& iFrameProperty) { m_frameProperty = iFrameProperty; }
    void SetFitMode(ui::FitMode2 const& iFitMode) { m_fitMode = iFitMode; }
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
    virtual ui::Component* VirtualAsComponent() override { return this; }
private:
    //void VirtualOnDraw(ui::DrawContext const& iContext)
    //{
    //    Common const& common = Common::Get();
    //    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    //    float const magnification = common.GetMagnifier().Magnification();

    //    box2f const& placementBox = PlacementBox();
    //    box2f const box = placementBox;
    //    ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
    //    if(AllGreaterStrict(box.Delta(), float2(0)))
    //    {
    //        drawer->DrawQuad(iContext, box + box2f::FromMinMax(float2(-2), float2(2)), Color4f(1,0,1,0) * 0.3f);
    //    }
    //    parent_type::VirtualOnDraw(iContext);
    //}
    virtual void VirtualUpdatePlacement() override;
private:
    safeptr<Magnifier const> m_magnifier;
    safeptr<Component> m_content;
    ui::FrameProperty m_frameProperty;
    ui::FitMode2 m_fitMode;
    float2 m_movableOffset;
};
//=============================================================================
}
}

#endif
