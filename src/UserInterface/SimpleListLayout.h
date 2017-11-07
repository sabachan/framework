#ifndef UserInterface_SimpleListLayout_H
#define UserInterface_SimpleListLayout_H

#include "Container.h"
#include "FitMode.h"
#include "FrameProperty.h"
#include "Length.h"
#include "Movable.h"
#include <Core/ArrayView.h>

// This optimisation is mainly useful for FitToContentOnly or
// FitToMinContentAndFrame modes.
#define SG_SIMPLE_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD 1

namespace sg {
namespace ui {
//=============================================================================
class Magnifier;
//=============================================================================
template <size_t direction>
class SimpleListLayout : public Container
                       , public IMovable
{
    static_assert(0 == direction || 1 == direction, "direction must be 0 or 1");
    static size_t const widthDirection = 1 - direction;
    PARENT_SAFE_COUNTABLE(Container)
public:
    struct Properties
    {
        struct
        {
            Length left;
            Length right;
            Length top;
            Length bottom;
            Length interItem;
        } margins;
        FitMode widthFitMode { FitMode::FitToFrameOnly };
    };
public:
    SimpleListLayout(Magnifier const& iMagnifier);
    SimpleListLayout(Magnifier const& iMagnifier, Properties const& iProperties);
    virtual ~SimpleListLayout() override;
    void AppendItem(IMovable* iItem);
    //void InsertItemAt(IMovable* iItem, size_t iPos);
    //void InsertItemAfter(IMovable* iItem, IMovable* iAfterWhichh);
    void RemoveItem(IMovable* iItem);
    void RemoveAllItems();
    //template <typename T>
    //void SortItems(T iComp);
    ArrayView<safeptr<IMovable> const> Items() const { return ArrayView<safeptr<IMovable> const>(m_items.data(), m_items.size()); }
protected:
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
    virtual void VirtualUpdatePlacement() override;
    virtual void VirtualOnChildInvalidatePlacement() override { InvalidatePlacement(); }
    virtual ui::Component* VirtualAsComponent() { return this; }
private:
#if SG_ENABLE_ASSERT
    void CheckConstraintsOnItem(IMovable* iItem);
#endif
private:
    safeptr<ui::Magnifier const> m_magnifier; // TODO: MagnifiableContainer, MagnifiableComponent
    std::vector<safeptr<IMovable>> m_items;
    safeptr<SubContainer> m_subContainer;
    Properties m_properties;
    float2 m_offset;
#if SG_SIMPLE_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD
    safeptr<IMovable> m_lastBiggestChild;
#endif
};
//=============================================================================
typedef SimpleListLayout<0> HorizontalSimpleListLayout;
typedef SimpleListLayout<1> VerticalSimpleListLayout;
//=============================================================================
}
}

#endif
