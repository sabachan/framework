#ifndef UserInterface_RootContainer_H
#define UserInterface_RootContainer_H

#include "Container.h"
#include "Focusable.h"

namespace sg {
namespace ui {
//=============================================================================
class RootContainer final : public Container, private IFocusable
{
    typedef Container parent_type;
public:
    RootContainer();
    ~RootContainer();
    void SetPlacementBox(box2f const& iBox);
private:
    virtual Component* VirtualAsComponent() override { return this; }
    virtual IFocusable* VirtualAsFocusableIFP() override { return this; }
    virtual void VirtualOnInsertInUI() override { parent_type::VirtualOnInsertInUI(); OnInsertFocusableInUI(); }
    virtual void VirtualOnRemoveFromUI() override { OnRemoveFocusableFromUI(); parent_type::VirtualOnRemoveFromUI(); }
private:
    virtual void VirtualUpdatePlacement() override;
};
//=============================================================================
}
}

#endif
