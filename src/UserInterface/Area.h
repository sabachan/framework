#ifndef UserInterface_Area_H
#define UserInterface_Area_H

#include <Core/BitSet.h>
#include <Math/Box.h>
#include <Math/Vector.h>


namespace sg {
namespace system {
class UserInputEvent;
}
}

namespace sg {
namespace ui {
//=============================================================================
class IArea;
class ISensitiveAreaListener;
class PointerEvent;
class PointerEventContext;
//=============================================================================
bool IsInside_AssumePointerEvent(PointerEventContext const& iContext, system::UserInputEvent const& iEvent, IArea const& iArea, float3& oPosition);
bool IsInside_AssumePointerEvent(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const& iArea, float3& oPosition);
//=============================================================================
class IArea
{
public:
    virtual bool VirtualIsInArea(float2 const& iPos) const = 0;
    bool IsInside(PointerEventContext const& iContext, PointerEvent const& iEvent, float3& oPosition);
};
//=============================================================================
class BoxArea : public IArea
{
public:
    BoxArea() : m_box() {}
    explicit BoxArea(box2f const& iBox) : m_box(iBox) {}
    virtual bool VirtualIsInArea(float2 const& iPos) const override { return m_box.Contains(iPos); }
    box2f const& Box() const { return m_box; }
    void SetBox(box2f const& iBox) { m_box = iBox; }
private:
    box2f m_box;
};
//=============================================================================
class AllArea : public IArea
{
public:
    virtual bool VirtualIsInArea(float2 const& iPos) const override { SG_UNUSED(iPos); return true; }
};
//=============================================================================
}
}

#endif
