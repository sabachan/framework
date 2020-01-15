#ifndef UserInterface_Movable_H
#define UserInterface_Movable_H

#include "Component.h"

namespace sg {
namespace ui {
//=============================================================================
class IMovable;
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
class IMovableHolder
{
public:
    IMovableHolder() {}
    IMovableHolder(IMovable* iMovable)
        : m_asComponent(nullptr == iMovable ? nullptr : iMovable->AsComponent())
        , m_asMovable(iMovable)
    {
    }
    IMovableHolder& operator= (IMovable* iMovable)
    {
        m_asMovable = iMovable;
        m_asComponent = nullptr == iMovable ? nullptr : iMovable->AsComponent();
        return *this;
    }

    IMovable* get() const { return m_asMovable.get(); }
    IMovable* operator->() const { return m_asMovable.get(); }
    IMovable& operator*() const { return *m_asMovable; }
private:
    refptr<Component> m_asComponent;
    safeptr<IMovable> m_asMovable;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T> inline bool operator==(IMovableHolder const& a, safeptr<T> const& b) { return a.get() == b.get(); }
template <typename T> inline bool operator!=(IMovableHolder const& a, safeptr<T> const& b) { return a.get() != b.get(); }
template <typename T> inline bool operator==(safeptr<T> const& a, IMovableHolder const& b) { return a.get() == b.get(); }
template <typename T> inline bool operator!=(safeptr<T> const& a, IMovableHolder const& b) { return a.get() != b.get(); }
template <typename T> inline bool operator==(IMovableHolder const& a, T const* b) { return a.get() == b; }
template <typename T> inline bool operator!=(IMovableHolder const& a, T const* b) { return a.get() != b; }
template <typename T> inline bool operator==(T const* a, IMovableHolder const& b) { return a == b.get(); }
template <typename T> inline bool operator!=(T const* a, IMovableHolder const& b) { return a != b.get(); }
inline bool operator==(IMovableHolder const& a, std::nullptr_t) { return nullptr == a.get(); }
inline bool operator!=(IMovableHolder const& a, std::nullptr_t) { return nullptr != a.get(); }
inline bool operator==(std::nullptr_t, IMovableHolder const& a) { return nullptr == a.get(); }
inline bool operator!=(std::nullptr_t, IMovableHolder const& a) { return nullptr != a.get(); }
//=============================================================================
}
}

#endif
