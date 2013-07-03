#ifndef UserInterface_Container_H
#define UserInterface_Container_H

#include "Component.h"
#include <vector>

namespace sg {
namespace ui {
//=============================================================================
class Container : public Component
{
    typedef Component parent_type;
    friend class Component;
    friend class RootContainer;
public:
    Container();
    virtual ~Container() override;
    void RequestAddToFront(Component* iComponent, i32 iLayer = 0);
    void RequestAddToBack(Component* iComponent, i32 iLayer = 0);
    void RequestMoveToFront(Component* iComponent);
    void RequestMoveToBack(Component* iComponent);
    void RequestMoveToFrontOfLayer(Component* iComponent, i32 iLayer);
    void RequestMoveToBackOfLayer(Component* iComponent, i32 iLayer);
    void RequestRemove(Component* iComponent);
    void RequestRemoveAll();
    bool Empty() const { return m_children.Empty() && m_requests.empty(); }
protected:
    virtual void VirtualOnDraw(DrawContext const& iContext) override;
    virtual void VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent) override;
    virtual void VirtualInvalidateChildrenPlacement();
    virtual void VirtualOnChildInvalidatePlacement();
    virtual void VirtualOnInsertInUI();
    virtual void VirtualOnRemoveFromUI();
private:
    Container(Component::IsRootTag);
    void AddToFront(Component* iComponent, i32 iLayer);
    void AddToBack(Component* iComponent, i32 iLayer);
    void MoveToFront(Component* iComponent, i32 iLayer);
    void MoveToBack(Component* iComponent, i32 iLayer);
    void Remove(Component* iComponent);
    void RemoveAll();
    void ExecuteRequests();
    void OnChildInvalidatePlacement();
private:
    IntrusiveList<Component> m_children;
    struct Request
    {
        enum class Type { AddToFront, AddToBack, MoveToFrontSameLayer, MoveToBackSameLayer, MoveToFront, MoveToBack, Remove, RemoveAll };
        Type type;
        safeptr<Component> component;
        i32 layer;
    public:
        Request(Type iType, Component* iComponent, i32 iLayer) : type(iType), component(iComponent), layer(iLayer) {}
    };
    std::vector<Request> m_requests;
    bool m_requestLocked;
};
//=============================================================================
class RootContainer final : public Container
{
    typedef Container parent_type;
public:
    RootContainer();
    ~RootContainer();
    void SetPlacementBox(box2f const& iBox);
private:
    virtual void VirtualUpdatePlacement() override;
};
//=============================================================================
// This sub container is useful to handle margins inside another container.
class SubContainer final : public Container
{
    typedef Container parent_type;
public:
    using Container::SetPlacementBox;
private:
    virtual void VirtualUpdatePlacement() override { SG_ASSERT_NOT_REACHED(); }
};
//=============================================================================
}
}

#endif
