#include "stdafx.h"

#include "Focusable.h"

#include "Container.h"

namespace sg {
namespace ui {
//=============================================================================
IFocusable::IFocusable()
    SG_CODE_FOR_ASSERT(: m_isRoot(false))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IFocusable::IFocusable(ComponentIsRoot_t)
    : m_focusParent(this)
    SG_CODE_FOR_ASSERT(SG_COMMA m_isRoot(true))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IFocusable::~IFocusable()
{
    SG_ASSERT_MSG(!m_isInGUI, "Did you forget a call to On(Insert/Remove)Focusable(In/From)UI ?");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::OnFocusableEvent(FocusableEvent const& iFocusableEvent)
{
    SG_ASSERT(HasFocus());
    SG_ASSERT(AsComponent()->IsInGUI());
    SG_ASSERT_MSG(AsComponent()->IsInGUI() == m_isInGUI, "Did you forget a call to On(Insert/Remove)Focusable(In/From)UI ?");
    SG_ASSERT(!m_virtualEventCalled);
    AsComponent()->UpdatePlacementIFN();
    VirtualOnFocusableEvent(iFocusableEvent);
    SG_ASSERT_MSG(m_virtualEventCalled, "Did you forget a call to parent::VirtualOnFocusableEvent");
    SG_CODE_FOR_ASSERT(m_virtualEventCalled = false;)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::VirtualOnFocusableEvent(FocusableEvent const& iFocusableEvent)
{
    if(nullptr != m_focusChild)
        m_focusChild->OnFocusableEvent(iFocusableEvent);
    SG_CODE_FOR_ASSERT(m_virtualEventCalled = true;)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool IFocusable::RequestMoveFocusReturnHasMoved(FocusDirection iDirection)
{
    SG_ASSERT(AsComponent()->IsInGUI());
    SG_ASSERT_MSG(AsComponent()->IsInGUI() == m_isInGUI, "Did you forget a call to On(Insert/Remove)Focusable(In/From)UI ?");
    AsComponent()->UpdatePlacementIFN();
    bool const hasMoved = VirtualMoveFocusReturnHasMoved(iDirection);
    return hasMoved;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool IFocusable::VirtualMoveFocusReturnHasMoved(FocusDirection iDirection)
{
    if(!HasFocus())
    {
        RequestFocusIFN();
        return true;
    }
    if(nullptr != m_focusChild)
        return m_focusChild->VirtualMoveFocusReturnHasMoved(iDirection);
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::RequestFocus()
{
    SG_ASSERT(!HasFocus());
    SG_ASSERT(AsComponent()->IsInGUI());
    SG_ASSERT_MSG(AsComponent()->IsInGUI() == m_isInGUI, "Did you forget a call to On(Insert/Remove)Focusable(In/From)UI ?");
    SG_ASSERT(!m_isRoot);
    Component* c = AsComponent();
    SG_ASSERT(c->IsInGUI());
    Container* p = c->Parent();
    SG_ASSERT(nullptr != p);
    IFocusable* f = p->AsFocusableIFP();
    while(nullptr == f)
    {
        p = p->Parent();
        SG_ASSERT(nullptr != p);
        f = p->AsFocusableIFP();
    }
    if(!f->HasFocus())
        f->RequestFocus();
    SG_ASSERT(f->HasFocus());
    f->SetFocusChild(this);
    SG_ASSERT(HasFocus());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::RequestFocusIFN()
{
    if(!HasFocus())
        RequestFocus();
    SG_ASSERT(HasFocus());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::RequestTerminalFocusIFN()
{
    if(!HasFocus())
        RequestFocus();
    else if(!HasTerminalFocus())
        SetFocusChild(nullptr);
    SG_ASSERT(HasTerminalFocus());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::ReleaseFocusIFN()
{
    if(HasFocus())
    {
        SG_ASSERT(nullptr != m_focusParent);
        m_focusParent->SetFocusChild(nullptr);
    }
    SG_ASSERT(!HasFocus() || m_isRoot);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::SetFocusChild(IFocusable* iFocusable)
{
    // SetFocusChild(nullptr) may be called in 2 cases:
    // - either on an explicit release of the child.
    //      In that case, the container still has the focus and may want to
    //      give it to another of its children or release it itself. Hence,
    //      VirtualOnSetFocusChild is called
    // - or when removing the focus from each nodes of a branch.
    //      In that case, we don't want VirtualOnSetFocusChild to be called.
    // To differentiate, we remove the parent before the childre when
    // unfocusing a branch, and VirtualOnSetFocusChild is only called when
    // effectively having the focus.
    // In the case iFocusable is non null, then we must already have the focus
    // and the virtual method is called.
    // Note however that the method is not reentrant, except on nullptr.
    SG_ASSERT(!m_isInSetFocusChild);
    SG_CODE_FOR_ASSERT(m_isInSetFocusChild = nullptr != iFocusable;)
    SG_ASSERT(HasFocus() || nullptr == iFocusable);
    if(nullptr != m_focusChild)
    {
        SG_ASSERT(this == m_focusChild->m_focusParent);
        m_focusChild->OnLooseFocus();
        m_focusChild->m_focusParent = nullptr;
        m_focusChild->SetFocusChild(nullptr);
        m_focusChild = nullptr;
    }
    if(nullptr != iFocusable)
    {
        SG_ASSERT(!iFocusable->HasFocus());
        SG_ASSERT(nullptr == iFocusable->m_focusParent);
        SG_ASSERT(nullptr == iFocusable->m_focusChild);
        m_focusChild = iFocusable;
        m_focusChild->m_focusParent = this;
        m_focusChild->OnGainFocus();
    }
    if(HasFocus())
        OnSetFocusChild(iFocusable);
    SG_CODE_FOR_ASSERT(if(nullptr != iFocusable) { m_isInSetFocusChild = false; })
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::OnInsertFocusableInUI()
{
    SG_ASSERT_MSG(AsComponent()->IsInGUI(), "OnInsertFocusableInUI should be call before VirtualOnInsertInUI");
    SG_CODE_FOR_ASSERT(m_isInGUI = true;)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::OnRemoveFocusableFromUI()
{
    SG_ASSERT_MSG(AsComponent()->IsInGUI(), "OnRemoveFocusableFromUI should be call before VirtualOnRemoveFromUI");
    ReleaseFocusIFN();
    SG_CODE_FOR_ASSERT(m_isInGUI = false;)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::OnSetFocusChild(IFocusable* iFocusable)
{
    SG_ASSERT(HasFocus());
    SG_ASSERT(nullptr != m_focusParent);
    SG_ASSERT(iFocusable == m_focusChild);
    VirtualOnSetFocusChild(iFocusable);
    SG_CODE_FOR_ASSERT(m_virtualSetChildCalled = false;)
    SG_ASSERT(HasFocus() || nullptr == iFocusable);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::VirtualOnSetFocusChild(IFocusable* iFocusable)
{
    SG_UNUSED(iFocusable);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::OnGainFocus()
{
    SG_ASSERT(HasFocus());
    VirtualOnGainFocus();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::OnLooseFocus()
{
    SG_ASSERT(HasFocus());
    VirtualOnLooseFocus();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::VirtualOnGainFocus()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void IFocusable::VirtualOnLooseFocus()
{
}
//=============================================================================
}
}
