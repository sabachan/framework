#include "stdafx.h"

#include "Container.h"

#include <Core/For.h>

namespace sg {
namespace ui {
//=============================================================================
Container::Container()
: parent_type()
, m_children()
, m_requests()
, m_requestLocked(false)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Container::Container(Component::IsRootTag tag)
: parent_type(tag)
, m_children()
, m_requests()
, m_requestLocked(false)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Container::~Container()
{
    RemoveAll();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::VirtualOnDraw(DrawContext const& iContext)
{
    SG_ASSERT(!m_requestLocked);
    SG_ASSERT(IsPlacementUpToDate());
#if 0
    // Unecessary, as Component does nothing (except for assert). Kept for doc.
    parent_type::VirtualOnDraw(iContext);
#else
#if SG_ENABLE_ASSERT
    SG_ASSERT(!parent_type::m_parentDrawOrPointerEventHasBeenCalled);
    parent_type::m_parentDrawOrPointerEventHasBeenCalled = true;
#endif
#endif
    SG_ASSERT(!m_requestLocked);
    m_requestLocked = true;
    for(Component& it : reverse_view(m_children))
        it.OnDraw(iContext);
    SG_ASSERT(m_requestLocked);
    m_requestLocked = false;
    ExecuteRequests();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent)
{
    SG_ASSERT(!m_requestLocked);
    SG_ASSERT(IsPlacementUpToDate());
#if 0
    // Unecessary, as Component does nothing (except for assert). Kept for doc.
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
#else
#if SG_ENABLE_ASSERT
    SG_ASSERT(!parent_type::m_parentDrawOrPointerEventHasBeenCalled);
    parent_type::m_parentDrawOrPointerEventHasBeenCalled = true;
#endif
#endif
    SG_ASSERT(!m_requestLocked);
    m_requestLocked = true;
    for(Component& it : m_children)
        it.OnPointerEvent(iContext, iPointerEvent);
    SG_ASSERT(m_requestLocked);
    m_requestLocked = false;
    ExecuteRequests();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::VirtualInvalidateChildrenPlacement()
{
    for(auto& it : m_children)
        it.InvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::VirtualOnChildInvalidatePlacement()
{
    if(nullptr != Parent())
        Parent()->OnChildInvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::OnChildInvalidatePlacement()
{
    SG_ASSERT(IsInGUI());
    if(!m_isInUpdatePlacement)
        VirtualOnChildInvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::VirtualOnInsertInUI()
{
    SG_ASSERT(!m_requestLocked);
    m_requestLocked = true;
    parent_type::VirtualOnInsertInUI();
    for(auto& it : m_children)
        it.OnInsertInUI();
    SG_ASSERT(m_requestLocked);
    m_requestLocked = false;
    ExecuteRequests();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::VirtualOnRemoveFromUI()
{
    SG_ASSERT(!m_requestLocked);
    m_requestLocked = true;
    for(auto& it : m_children)
        it.OnRemoveFromUI();
    parent_type::VirtualOnRemoveFromUI();
    SG_ASSERT(m_requestLocked);
    m_requestLocked = false;
    ExecuteRequests();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::RequestAddToFront(Component* iComponent, i32 iLayer)
{
    if(m_requestLocked)
        m_requests.emplace_back(Request::Type::AddToFront, iComponent, iLayer);
    else
        AddToFront(iComponent, iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::RequestAddToBack(Component* iComponent, i32 iLayer)
{
    if(m_requestLocked)
        m_requests.emplace_back(Request::Type::AddToBack, iComponent, iLayer);
    else
        AddToBack(iComponent, iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::RequestMoveToFront(Component* iComponent)
{
    if(m_requestLocked)
        m_requests.emplace_back(Request::Type::MoveToFrontSameLayer, iComponent, 0);
    else
    {
        i32 const layer = iComponent->LayerInContainer();
        MoveToFront(iComponent, layer);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::RequestMoveToBack(Component* iComponent)
{
    if(m_requestLocked)
        m_requests.emplace_back(Request::Type::MoveToBackSameLayer, iComponent, 0);
    else
    {
        i32 const layer = iComponent->LayerInContainer();
        MoveToBack(iComponent, layer);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::RequestMoveToFrontOfLayer(Component* iComponent, i32 iLayer)
{
    if(m_requestLocked)
        m_requests.emplace_back(Request::Type::MoveToFront, iComponent, iLayer);
    else
        MoveToFront(iComponent, iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::RequestMoveToBackOfLayer(Component* iComponent, i32 iLayer)
{
    if(m_requestLocked)
        m_requests.emplace_back(Request::Type::MoveToBack, iComponent, iLayer);
    else
        MoveToBack(iComponent, iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::RequestRemove(Component* iComponent)
{
    if(m_requestLocked)
        m_requests.emplace_back(Request::Type::Remove, iComponent, 0);
    else
        Remove(iComponent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::RequestRemoveAll()
{
    if(m_requestLocked)
        m_requests.emplace_back(Request::Type::RemoveAll, nullptr, 0);
    else
        RemoveAll();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::AddToFront(Component* iComponent, i32 iLayer)
{
    SG_ASSERT(!m_requestLocked);
    SG_ASSERT(nullptr != iComponent);
    SG_ASSERT(nullptr == iComponent->Parent());
    auto it = m_children.begin();
    auto end = m_children.end();
    while(it != end && it->LayerInContainer() > iLayer) { ++it; }
    if(it != end) m_children.InsertBefore(it.get(), iComponent);
    else m_children.PushBack(iComponent);
    iComponent->SetParent(this, iLayer);
    if(IsInGUI())
        iComponent->OnInsertInUI();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::AddToBack(Component* iComponent, i32 iLayer)
{
    SG_ASSERT(!m_requestLocked);
    SG_ASSERT(nullptr != iComponent);
    SG_ASSERT(nullptr == iComponent->Parent());
    auto it = m_children.rbegin();
    auto end = m_children.rend();
    while(it != end && it->LayerInContainer() < iLayer) { ++it; }
    if(it != end) m_children.InsertAfter(it.get(), iComponent);
    else m_children.PushFront(iComponent);
    iComponent->SetParent(this, iLayer);
    if(IsInGUI())
        iComponent->OnInsertInUI();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::MoveToFront(Component* iComponent, i32 iLayer)
{
    SG_ASSERT(!m_requestLocked);
    SG_ASSERT(nullptr != iComponent);
    SG_ASSERT(this == iComponent->Parent());
    auto it = m_children.begin();
    auto end = m_children.end();
    while(it != end && it->LayerInContainer() > iLayer) { ++it; }
    if(it.get() != iComponent)
    {
        if(it != end) m_children.MoveToBefore(it.get(), iComponent);
        else m_children.MoveToBack(iComponent);
    }
    iComponent->SetLayerInContainer(this, iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::MoveToBack(Component* iComponent, i32 iLayer)
{
    SG_ASSERT(!m_requestLocked);
    SG_ASSERT(nullptr != iComponent);
    SG_ASSERT(this == iComponent->Parent());
    auto it = m_children.rbegin();
    auto end = m_children.rend();
    while(it != end && it->LayerInContainer() < iLayer) { ++it; }
    if(it.get() != iComponent)
    {
        if(it != end) m_children.MoveToAfter(it.get(), iComponent);
        else m_children.MoveToFront(iComponent);
    }
    iComponent->SetLayerInContainer(this, iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::Remove(Component* iComponent)
{
    SG_ASSERT(!m_requestLocked);
    SG_ASSERT(nullptr != iComponent);
    SG_ASSERT(this == iComponent->Parent());
    if(IsInGUI())
        iComponent->OnRemoveFromUI();
    iComponent->SetParent(nullptr, 0);
    m_children.Remove(iComponent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::RemoveAll()
{
    SG_ASSERT(!m_requestLocked);
    for(Component& it : m_children)
    {
        if(IsInGUI())
            it.OnRemoveFromUI();
        it.SetParent(nullptr, 0);
    }
    m_children.Clear();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Container::ExecuteRequests()
{
    for(auto& r : m_requests)
    {
        switch(r.type)
        {
        case Request::Type::AddToFront:
            AddToFront(r.component.get(), r.layer);
            break;
        case Request::Type::AddToBack:
            AddToBack(r.component.get(), r.layer);
            break;
        case Request::Type::MoveToFrontSameLayer:
            MoveToFront(r.component.get(), r.component->LayerInContainer());
            break;
        case Request::Type::MoveToBackSameLayer:
            MoveToBack(r.component.get(), r.component->LayerInContainer());
            break;
        case Request::Type::MoveToFront:
            MoveToFront(r.component.get(), r.layer);
            break;
        case Request::Type::MoveToBack:
            MoveToBack(r.component.get(), r.layer);
            break;
        case Request::Type::Remove:
        {
            SG_ASSERT(0 == r.layer);
            Component* c = r.component.get();
            r.component = nullptr;
            Remove(c);
            break;
        }
        case Request::Type::RemoveAll:
            SG_ASSERT(nullptr == r.component);
            SG_ASSERT(0 == r.layer);
            RemoveAll();
            break;
        default:
            SG_ASSERT_NOT_REACHED();
        }
        r.component = nullptr;
    }
    m_requests.clear();
}
//=============================================================================
RootContainer::RootContainer()
: parent_type(Component::IsRootTag())
{
    OnInsertInUI();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RootContainer::~RootContainer()
{
    OnRemoveFromUI();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RootContainer::SetPlacementBox(box2f const& iBox)
{
    parent_type::SetPlacementBox(iBox);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RootContainer::VirtualUpdatePlacement()
{
    SG_ASSERT_NOT_REACHED();
}
//=============================================================================
}
}
