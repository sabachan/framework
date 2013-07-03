#include "stdafx.h"

#include <Core/Config.h>

#include "ScrollingContainer.h"

#include "FitMode.h"
#include "GenericStyleGuide.h"
#include "Magnifier.h"
#include "Movable.h"
#include "PointerEvent.h"
#include "SensitiveArea.h"
#include "WheelHelper.h"
#include <System/MouseUtils.h>
#include <System/UserInputEvent.h>

namespace sg {
namespace ui {
//=============================================================================
class ScrollingContainer_SubContainer : public Container
                                      , private ISensitiveAreaListener
                                      , private IWheelListener
{
    typedef Container parent_type;
public:
    ScrollingContainer_SubContainer();
    virtual ~ScrollingContainer_SubContainer();
    using Container::SetPlacementBox;
private:
    virtual void VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent) override;
    virtual void OnWheelEvent(SG_UI_WHEEL_LISTENER_PARAMETERS) override;
    virtual void OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnPointerMoveOutsideButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    void OnPointerMoveImpl(float3 const& iPointerLocalPosition, size_t iButton);
    ScrollingContainer* GetScrollingContainer() { SG_ASSERT(IsInGUI()); return checked_cast<ScrollingContainer*>(Parent()); }
    virtual void VirtualUpdatePlacement() override { SG_ASSUME_NOT_REACHED(); }
private:
    SensitiveArea m_sensitiveArea;
    float2 m_previousPointerLocalPosition;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ScrollingContainer_SubContainer::ScrollingContainer_SubContainer()
    : m_sensitiveArea()
    , m_previousPointerLocalPosition()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ScrollingContainer_SubContainer::~ScrollingContainer_SubContainer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer_SubContainer::VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent)
{
    ScrollingContainer* parent = GetScrollingContainer();
    ui::BoxArea wheelArea(parent->PlacementBox_AssumeUpToDate());
    ui::BoxArea area(PlacementBox_AssumeUpToDate());

    ui::SensitiveArea::PremaskState premaskState(system::MouseEntry::Button2);
    m_sensitiveArea.OnPointerEventPreChildren(iContext, iPointerEvent, area, premaskState);
    ui::WheelHelper wheelHelper;
    wheelHelper.OnPointerEventPreChildren(iContext, iPointerEvent, wheelArea);
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
    wheelHelper.OnPointerEventPostChildren(iContext, iPointerEvent, *this);
    m_sensitiveArea.OnPointerEventPostChildren(iContext, iPointerEvent, premaskState, *this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer_SubContainer::OnWheelEvent(SG_UI_WHEEL_LISTENER_PARAMETERS)
{
    SG_UI_WHEEL_LISTENER_UNUSED_PARAMETERS;
    ScrollingContainer* parent = GetScrollingContainer();
    float const magnification = parent->GetMagnifier().Magnification();
    box2f const box = PlacementBox_AssumeUpToDate();
    ScrollingContainer::Properties const& properties = parent->GetProperties();
    float const scrollingSpeed = properties.scrollingSpeed.Resolve(magnification, box.Delta().y());
    float2 const offset = float2(0, scrollingSpeed * iWheelDelta);
    parent->ScrollInUnitIFP(offset);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer_SubContainer::OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON;
    SG_ASSERT(1 == iPointerLocalPosition.z());
    if(iButton == 2)
        m_previousPointerLocalPosition = iPointerLocalPosition.xy();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer_SubContainer::OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON;
    OnPointerMoveImpl(iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer_SubContainer::OnPointerMoveOutsideButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON;
    OnPointerMoveImpl(iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer_SubContainer::OnPointerMoveImpl(float3 const& iPointerLocalPosition, size_t iButton)
{
    SG_ASSERT(1 == iPointerLocalPosition.z());
    if(iButton == 2)
    {
        ScrollingContainer* parent = GetScrollingContainer();
        float2 const delta = iPointerLocalPosition.xy() - m_previousPointerLocalPosition;
        parent->ScrollInUnitIFP(delta);
        UpdatePlacementIFN();
        m_previousPointerLocalPosition = iPointerLocalPosition.xy();
    }
}
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
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ScrollingContainer::ScrollingContainer(Magnifier const& iMagnifier, TextureDrawer const* iTextureDrawer, Properties const& iProperties)
    : IMagnifiable(iMagnifier)
    , m_subContainer()
    , m_clippingContainer()
    , m_content()
    , m_properties(iProperties)
    , m_relativeScrollPosition(0)
    , m_barsMargins(box2f::FromMinMax(float2(0), float2(0)))
    , m_visibleBars(false)

{
    m_subContainer = new ScrollingContainer_SubContainer;
    m_clippingContainer = new ui::ClippingContainer(iTextureDrawer);
    m_subContainer->RequestAddToBack(m_clippingContainer.get());
    this->RequestAddToBack(m_subContainer.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ScrollingContainer::~ScrollingContainer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::SetContent(ui::IMovable* iContent)
{
    SG_ASSERT((nullptr == iContent) != (nullptr == m_content));
    if(nullptr != m_content)
    {
        Component* c = m_content->AsComponent();
        m_content = nullptr;
        m_clippingContainer->RequestRemove(c);
    }
    if(nullptr != iContent)
    {
        m_content = iContent;
#if SG_ENABLE_ASSERT
        refptr<ui::Component> stayAlive = iContent->AsComponent();
        CheckConstraintsOnItem(iContent);
#endif
        m_clippingContainer->RequestAddToFront(iContent->AsComponent());
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::VirtualUpdatePlacement()
{
    float const magnification = Magnification();

    ui::Container* parent = Parent();
    box2f const& parentBox = parent->PlacementBox_AssumeUpToDate();
    SG_ASSERT_MSG(nullptr != m_content, "A scrolling container should always have a content");
    m_content->ResetOffset();

    bool2 const prevVisibleBars = m_visibleBars;

    auto ComputeAndSetPlacementBox = [&]() {
        box2f const windowBox = parentBox + m_barsMargins;
        if(m_properties.fitMode.DoesNeedContentSize())
        {
            box2f const testBox = m_properties.fitMode.ComputeTestBox(windowBox);
            SetPlacementBox(parentBox);
            m_subContainer->SetPlacementBox(testBox);
            box2f const& contentBox = m_content->AsComponent()->PlacementBox();
            box2f const box = box2f::FromMinDelta(parentBox.Min(), m_properties.fitMode.ComputeFittedSize(windowBox.Delta(), contentBox.Delta()));
            SetPlacementBox(box - m_barsMargins);
            m_subContainer->SetPlacementBox(box);
        }
        else
        {
            SetPlacementBox(parentBox);
            m_subContainer->SetPlacementBox(windowBox);
        }
    };

    ComputeAndSetPlacementBox();

    for(;;)
    {
        box2f windowBox;
        box2f contentBox;
        GetWindowAndContentBox(windowBox, contentBox);
        float2 const scrollableDelta = contentBox.Delta() - windowBox.Delta();
        bool2 visibleBars = bool2(scrollableDelta.x() > 0, scrollableDelta.y() > 0);
        if(visibleBars == m_visibleBars)
            break;
        else
        {
            m_visibleBars = visibleBars;
            box2f barsMargins;
            VirtualSetVisibleBarsAndGetBarsMargins(m_visibleBars, barsMargins);
            m_barsMargins = barsMargins;

            ComputeAndSetPlacementBox();

            if(visibleBars == prevVisibleBars)
                break;
        }
    }

    m_relativeScrollPosition = saturate(m_relativeScrollPosition);

    box2f windowBox;
    box2f contentBox;
    GetWindowAndContentBox(windowBox, contentBox);
    float2 const scrollableDelta = contentBox.Delta() - windowBox.Delta();
    float2 const contentOwnOffset = contentBox.Min() - windowBox.Min();
    box2f const contentMargins = GetContentMargins();
    float2 offset = float2(0);
    if(m_visibleBars.x())
        offset.x() = contentMargins.Min().x() - m_relativeScrollPosition.x() * scrollableDelta.x() - contentOwnOffset.x();
    if(m_visibleBars.y())
        offset.y() = contentMargins.Min().y() - m_relativeScrollPosition.y() * scrollableDelta.y() - contentOwnOffset.y();

    if(float2(0) != offset)
        m_content->AddOffset(offset);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
box2f ScrollingContainer::GetContentMargins() const
{
    float const magnification = Magnification();
    box2f const& box = PlacementBox_AssumeUpToDate();
    float2 const boxDelta = box.Delta();
    box2f const& contentMargins = m_properties.scrollingMargins.Resolve(magnification, boxDelta);
    return contentMargins;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::GetWindowAndContentBox(box2f& oWindowBox, box2f& oContentBox) const
{
    oWindowBox = m_subContainer->PlacementBox();
    box2f const contentMargins = GetContentMargins();
    box2f const& contentBox = m_content->AsComponent()->PlacementBox();
    oContentBox = contentBox + contentMargins;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
box2f ScrollingContainer::GetRelativeWindowOnContent() const
{
    box2f windowBox;
    box2f contentBox;
    GetWindowAndContentBox(windowBox, contentBox);
    float2 const contentDelta = contentBox.Delta();
    box2f const relativeWindowOnContent = (windowBox - contentBox.Min()) / contentDelta;
    return relativeWindowOnContent;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::ScrollInUnitIFP(float2 const& iOffset)
{
    box2f windowBox;
    box2f contentBox;
    GetWindowAndContentBox(windowBox, contentBox);
    float2 const scrollableDelta = contentBox.Delta() - windowBox.Delta();
    float2 const relativeOffset = -iOffset * (1.f / componentwise::max(scrollableDelta, float2(1.f)));
    m_relativeScrollPosition += relativeOffset;
    InvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::ScrollToBoxInContentIFP(box2f const& iBoxInContent)
{
    SG_UNUSED(iBoxInContent);
    SG_ASSERT_NOT_IMPLEMENTED();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::VirtualSetVisibleBarsAndGetBarsMargins(bool2 iVisibleBars, box2f& oBarsMargins)
{
    SG_UNUSED(iVisibleBars);
    SG_ASSERT_NOT_IMPLEMENTED();
    oBarsMargins = box2f::FromMinMax(float2(0), float2(0));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ScrollingContainer::SetBarsMargins(box2f const& iBarsMargins)
{
    m_barsMargins = iBarsMargins;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
void ScrollingContainer::CheckConstraintsOnItem(ui::IMovable* iContent)
{
    ui::Component* c = iContent->AsComponent();
    SG_ASSERT(c->IsRefCounted_ForAssert());
    if(nullptr != c->Parent())
        return;

    ui::RootContainer testContainer;
    testContainer.RequestAddToFront(c);
    SG_ASSERT(c->Parent() == &testContainer);
    iContent->ResetOffset();
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(0)));
    box2f const box0 = c->PlacementBox();
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), 0.5f*box0.Delta()));
    box2f const box1 = c->PlacementBox();
    SG_ASSERT_MSG(box1.Delta() == box0.Delta(),
               "The minimum box of the content should be constant when the parent is smaller along the 2 dimensions.");
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), box0.Delta()));
    box2f const box2 = c->PlacementBox();
    SG_ASSERT_MSG(box2.Delta() == box0.Delta(),
               "The minimum box of the content should be constant when the parent is smaller along the 2 dimensions.");

    testContainer.RequestRemove(c);
    SG_ASSERT(nullptr == c->Parent());
}
#endif
//=============================================================================
}
}
