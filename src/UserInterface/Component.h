#ifndef UserInterface_Component_H
#define UserInterface_Component_H

#include <Core/IntrusiveList.h>
#include <Core/SmartPtr.h>
#include <Math/Box.h>

namespace sg {
namespace ui {
//=============================================================================
class Container;
class DrawContext;
class PointerEvent;
class PointerEventContext;
//=============================================================================
class Component : public RefCountable
                , public IntrusiveListRefCountableElem<Component>
{
    SG_NON_COPYABLE(Component);
    friend class Container;
    friend class RootContainer;
protected:
    Component();
public:
    virtual ~Component();

    void OnDraw(DrawContext const& iContext);
    void OnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent);
    void UpdatePlacementIFN();
    void InvalidatePlacement();
    box2f const& PlacementBox();
    box2f const& PlacementBox_AssumeUpToDate() const;
    bool IsInGUI() const;
    Container* Parent() const;
    void MoveToFrontOfAllUI();
    void SendResetPointerEvent();
protected:
    box2f const& PlacementBoxAsIs_AssumeInUpdatePlacement() const;
    void SetPlacementBox(box2f const& iBox);
    virtual void VirtualUpdatePlacement();
    virtual void VirtualOnChildInvalidatePlacement();
    virtual void VirtualInvalidateChildrenPlacement();
    virtual void VirtualOnDraw(DrawContext const& iContext);
    virtual void VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent);
    virtual void VirtualOnInsertInUI();
    virtual void VirtualOnRemoveFromUI();
#if SG_ENABLE_ASSERT
    void CheckParentDrawAndPointerEventAreCalled(bool iValue) { m_checkParentDrawAndPointerEventAreCalled = iValue; }
#endif
private:
    bool IsPlacementUpToDate() const;
    void ForceInvalidatePlacement();
    void OnInsertInUI();
    void OnRemoveFromUI();
    void SetParent(Container* iContainer, i32 iLayerInContainer);
    void SetLayerInContainer(Container* iContainer, i32 iLayerInContainer);
    i32 LayerInContainer() const { return m_layerInContainer; }
    struct IsRootTag {};
    Component(IsRootTag);
private:
    safeptr<Container> m_parent;
    box2f m_placementBox;
    i32 m_layerInContainer;
    bool const m_isRoot;
    bool m_isPlacementUpToDate;
    bool m_isInGUI;
    bool m_isInDrawOrPointerEvent;
    bool m_isInUpdatePlacement;
#if SG_ENABLE_ASSERT
    bool m_checkParentDrawAndPointerEventAreCalled;
    bool m_parentDrawOrPointerEventHasBeenCalled;
#endif
};
//=============================================================================
}
}

#endif
