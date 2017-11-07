#ifndef UserInterface_Movable_H
#define UserInterface_Movable_H

#include "Component.h"

namespace sg {
namespace ui {
//=============================================================================
class IMovable : public SafeCountable
{
public:
    virtual ~IMovable() {}
    virtual void ResetOffset() { InvalidatePlacementIFN(); VirtualResetOffset(); }
    virtual void AddOffset(float2 const& iOffset) { InvalidatePlacementIFN(); VirtualAddOffset(iOffset); }
    virtual void SetOffset(float2 const& iOffset) { InvalidatePlacementIFN(); VirtualResetOffset(); VirtualAddOffset(iOffset); }
    Component* AsComponent() { Component* c = VirtualAsComponent(); SG_ASSERT(nullptr != c); return c; }
protected:
    virtual Component* VirtualAsComponent() = 0;
    virtual void VirtualResetOffset() = 0;
    virtual void VirtualAddOffset(float2 const& iOffset) = 0;
private:
    void InvalidatePlacementIFN() { ui::Component* c = AsComponent(); if(c->IsInGUI()) c->InvalidatePlacement(); }

};
//=============================================================================
}
}

#endif
