#include "stdafx.h"

#include "Component.h"

#include "Container.h"
#include "Context.h"
#include "PointerEvent.h"

namespace sg {
namespace ui {
//=============================================================================
Component::Component()
: m_parent()
, m_placementBox()
, m_layerInContainer(0)
, m_isRoot(false)
, m_isPlacementUpToDate(false)
, m_isInGUI(false)
, m_isInDrawOrPointerEvent(false)
, m_isInUpdatePlacement(false)
#if SG_ENABLE_ASSERT
, m_checkParentDrawAndPointerEventAreCalled(true)
, m_parentDrawOrPointerEventHasBeenCalled(false)
#endif
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Component::Component(IsRootTag)
: m_parent()
, m_placementBox()
, m_layerInContainer(0)
, m_isRoot(true)
, m_isPlacementUpToDate(false)
, m_isInGUI(false)
, m_isInDrawOrPointerEvent(false)
, m_isInUpdatePlacement(false)
#if SG_ENABLE_ASSERT
, m_checkParentDrawAndPointerEventAreCalled(true)
, m_parentDrawOrPointerEventHasBeenCalled(false)
#endif
{

}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Component::~Component()
{
    SG_ASSERT(!IsInGUI());
    SG_ASSERT(nullptr == m_parent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::OnDraw(DrawContext const& iContext)
{
    SG_ASSERT(IsInGUI());
    SG_ASSERT(!m_parentDrawOrPointerEventHasBeenCalled);
    SG_ASSERT(!m_isInDrawOrPointerEvent);
    UpdatePlacementIFN();
    m_isInDrawOrPointerEvent = true;
    VirtualOnDraw(iContext);
    SG_ASSERT(m_isInDrawOrPointerEvent);
    m_isInDrawOrPointerEvent = false;
#if SG_ENABLE_ASSERT
    if(m_checkParentDrawAndPointerEventAreCalled)
        SG_ASSERT_MSG(m_parentDrawOrPointerEventHasBeenCalled, "did you forget a call to parent_type::VirtualOn[Draw|PointerEvent]() ?");
    m_parentDrawOrPointerEventHasBeenCalled = false;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::OnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent)
{
    SG_ASSERT(IsInGUI());
    SG_ASSERT(!m_parentDrawOrPointerEventHasBeenCalled);
    SG_ASSERT(!m_isInDrawOrPointerEvent);
    UpdatePlacementIFN();
    m_isInDrawOrPointerEvent = true;
    VirtualOnPointerEvent(iContext, iPointerEvent);
    SG_ASSERT(m_isInDrawOrPointerEvent);
    m_isInDrawOrPointerEvent = false;
#if SG_ENABLE_ASSERT
    if(m_checkParentDrawAndPointerEventAreCalled)
        SG_ASSERT_MSG(m_parentDrawOrPointerEventHasBeenCalled, "did you forget a call to parent_type::VirtualOn[Draw|PointerEvent]() ?");
    m_parentDrawOrPointerEventHasBeenCalled = false;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::UpdatePlacementIFN()
{
    SG_ASSERT(IsInGUI());
    if(m_isInDrawOrPointerEvent)
    {
        // NB: When m_isInDrawOrPointerEvent, we want the placement to be
        // constant for all children. Hence we will keep current placement.
        return;
    }
    if(m_isInUpdatePlacement)
    {
        SG_ASSERT(m_isPlacementUpToDate);
        return;
    }
    SG_ASSERT(nullptr != m_parent || m_isRoot);
    if(nullptr != m_parent)
        Parent()->UpdatePlacementIFN();
    SG_ASSERT(!m_isInUpdatePlacement);
    if(!m_isPlacementUpToDate)
    {
        m_isInUpdatePlacement = true;
        VirtualUpdatePlacement();
        SG_ASSERT(m_isPlacementUpToDate);
        SG_ASSERT(m_isInUpdatePlacement);
        m_isInUpdatePlacement = false;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::InvalidatePlacement()
{
    if(!IsInGUI())
        return;
    SG_ASSERT(nullptr != Parent() || m_isRoot);
    // if component is already in UpdatePlacement(), there is no need to inform
    // its parent, which has already been informed when it was invalidated for
    // the firts time.
    // However, component is invalidated to be sure that it will take this
    // invalidation into consideration and will set its placement in the end.
    if(m_isPlacementUpToDate)
    {
        m_isPlacementUpToDate = false;
        if(!m_isInUpdatePlacement && !m_isRoot)
            Parent()->OnChildInvalidatePlacement();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::ForceInvalidatePlacement()
{
    SG_ASSERT(IsInGUI());
    SG_ASSERT(nullptr != Parent() || m_isRoot);
    m_isPlacementUpToDate = false;
    if(!m_isRoot)
        Parent()->OnChildInvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
box2f const& Component::PlacementBox()
{
    UpdatePlacementIFN();
    return PlacementBox_AssumeUpToDate();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
box2f const& Component::PlacementBox_AssumeUpToDate() const
{
    // NB: When m_isInDrawOrPointerEvent, we want the placement to be constant
    // for all children. Hence we will keep current placement.
    SG_ASSERT(m_isPlacementUpToDate || m_isInUpdatePlacement || m_isInDrawOrPointerEvent);
    return m_placementBox;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
box2f const& Component::PlacementBoxAsIs_AssumeInUpdatePlacement() const
{
    SG_ASSERT(m_isInUpdatePlacement);
    return m_placementBox;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::SetPlacementBox(box2f const& iBox)
{
    bool const saved_isInUpdatePlacement = m_isInUpdatePlacement;
    m_isInUpdatePlacement = true;
    m_placementBox = iBox;
    VirtualInvalidateChildrenPlacement();
    m_isPlacementUpToDate = true;
    m_isInUpdatePlacement = saved_isInUpdatePlacement;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Component::IsInGUI() const
{
    return m_isInGUI;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Container* Component::Parent() const
{
    return m_parent.get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::MoveToFrontOfAllUI()
{
    SG_ASSERT(IsInGUI());
    SG_ASSERT(m_isRoot || nullptr != Parent());
    if(!m_isRoot)
    {
        Parent()->RequestMoveToFront(this);
        Parent()->MoveToFrontOfAllUI();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::VirtualUpdatePlacement()
{
    SG_ASSERT(IsInGUI());
    SG_ASSERT(nullptr != m_parent);
    SetPlacementBox(m_parent->PlacementBox_AssumeUpToDate());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::VirtualOnChildInvalidatePlacement()
{
    // nothing to do
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::VirtualInvalidateChildrenPlacement()
{
    // nothing to do
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::VirtualOnDraw(DrawContext const& iContext)
{
    SG_UNUSED(iContext);
#if SG_ENABLE_ASSERT
    SG_ASSERT(!m_parentDrawOrPointerEventHasBeenCalled);
    m_parentDrawOrPointerEventHasBeenCalled = true;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent)
{
    SG_UNUSED((iContext, iPointerEvent));
#if SG_ENABLE_ASSERT
    SG_ASSERT(!m_parentDrawOrPointerEventHasBeenCalled);
    m_parentDrawOrPointerEventHasBeenCalled = true;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::VirtualOnInsertInUI()
{
    SG_ASSERT(!m_isInGUI);
    m_isInGUI = true;
    ForceInvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::VirtualOnRemoveFromUI()
{
    SG_ASSERT(m_isInGUI);
    ForceInvalidatePlacement();
    m_isInGUI = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Component::IsPlacementUpToDate() const
{
    SG_ASSERT(m_isInGUI);
    return m_isPlacementUpToDate || m_isInDrawOrPointerEvent;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::OnInsertInUI()
{
    SG_ASSERT(!m_isInGUI);
    VirtualOnInsertInUI();
    SG_ASSERT_MSG(m_isInGUI, "Did you forget a call to parent_type::VirtualOnInsertInUI()?");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::OnRemoveFromUI()
{
    SG_ASSERT(m_isInGUI);
    // NB: OnRemoveFromUI can do a lot of things, especially sending the reset
    // event that may need some placement boxes. Hence, we do things as in
    // OnDraw or OnPointerEvent.
    UpdatePlacementIFN();
    SendResetPointerEvent();
    SG_ASSERT(!m_isInDrawOrPointerEvent);
    m_isInDrawOrPointerEvent = true;
    VirtualOnRemoveFromUI();
    SG_ASSERT(m_isInDrawOrPointerEvent);
    m_isInDrawOrPointerEvent = false;
    SG_ASSERT_MSG(!m_isInGUI, "Did you forget a call to parent_type::VirtualOnRemoveFromUI()?");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::SetParent(Container* iContainer, i32 iLayerInContainer)
{
    SG_ASSERT(nullptr == iContainer || nullptr == m_parent);
    SG_ASSERT(nullptr != iContainer || 0 == iLayerInContainer);
    m_parent = iContainer;
    m_layerInContainer = iLayerInContainer;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::SetLayerInContainer(Container* iContainer, i32 iLayerInContainer)
{
    SG_ASSERT(nullptr != m_parent);
    SG_ASSERT_AND_UNUSED(m_parent == iContainer);
    m_layerInContainer = iLayerInContainer;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Component::SendResetPointerEvent()
{
    system::UserInputEvent resetEvent(system::reset_all);
    PointerEvent pointerEvent(resetEvent SG_CODE_FOR_ASSERT(SG_COMMA 0));
    PointerEventContext context;
    OnPointerEvent(context, pointerEvent);
}
//=============================================================================
}
}
