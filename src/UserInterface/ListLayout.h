#ifndef UserInterface_ListLayout_H
#define UserInterface_ListLayout_H

#include "Container.h"
#include "FitMode.h"
#include "FrameProperty.h"
#include "Length.h"
#include "Movable.h"
#include <Core/ArrayView.h>

// This optimisation is mainly useful for FitToContentOnly or
// FitToMinContentAndFrame modes.
#define SG_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD 0 //1

namespace sg {
namespace ui {
//=============================================================================
class Magnifier;
//=============================================================================
template <size_t direction>
class ListLayout : public Container
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
    ListLayout(Magnifier const& iMagnifier);
    ListLayout(Magnifier const& iMagnifier, Properties const& iProperties);
    virtual ~ListLayout() override;
    void AppendItem(IMovable* iItem) { PrivateAppendItem(iItem, nullptr); }
    void AppendExpansibleItem(Component* iItem, Length iMinLength, float iWeight);
    //void InsertItemAt(IMovable* iItem, size_t iPos);
    //void InsertItemAfter(IMovable* iItem, IMovable* iAfterWhichh);
    void RemoveItem(Component* iItem);
    void RemoveAllItems();
    //template <typename T>
    //void SortItems(T iComp);
    //ArrayView<safeptr<IMovable> const> Items() const { return ArrayView<safeptr<IMovable> const>(m_items.data(), m_items.size()); }
    Magnifier const* GetMagnifier() { return m_magnifier.get(); }
protected:
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
    virtual void VirtualUpdatePlacement() override;
    virtual void VirtualOnChildInvalidatePlacement() override { InvalidatePlacement(); }
    virtual ui::Component* AsComponent() { return this; }
private:
#if SG_ENABLE_ASSERT
    void CheckConstraintsOnItem(IMovable* iItem);
#endif
private:
    class Cell : public Container
               , public IMovable
    {
        PARENT_SAFE_COUNTABLE(Container)
    public:
        //Cell(Magnifier const& iMagnifier, Component* iContent, Length iMinLength, float iWeight);
        Cell(Component* iContent, Length iMinLength, float iWeight);
    private:
        friend ListLayout;
        //void ResetExpandedLength() { m_expandedLength = 0; }
        //void SetExpandedLength(float iToAdd) { m_expandedLength += iToAdd; }
        //void ResolveMinLength(float iMagnification);
        virtual void VirtualResetOffset() override { m_offset = float2(0); }
        virtual void VirtualAddOffset(float2 const& iOffset) override { m_offset += iOffset; }
        virtual void VirtualUpdatePlacement() override;
        virtual ui::Component* AsComponent() { return this; }
    private:
        //safeptr<ui::Magnifier const> m_magnifier;
        safeptr<Component> m_content;
        Length const m_minLength;
        float const m_weight;
        float m_resolvedMinLength;
        float m_requestedLength;
        float2 m_offset;
        bool m_isConstrained;
    };
    struct Item
    {
        safeptr<IMovable> asMovable;
        safeptr<Cell> asCell;
        Item(IMovable* iAsMovable, Cell* iAsCell) : asMovable(iAsMovable), asCell(iAsCell) {}
    };
private:
    void PrivateAppendItem(IMovable* iAsMovable, Cell* iAsCell);
private:
    safeptr<ui::Magnifier const> m_magnifier; // TODO: MagnifiableContainer, MagnifiableComponent
    std::vector<Item> m_items;
    //std::vector<u8> m_isCell;
    safeptr<SubContainer> m_subContainer;
    Properties m_properties;
    float2 m_offset;
#if SG_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD
    size_t m_lastBiggestChildIndex;
    //safeptr<IMovable> m_lastBiggestChild;
#endif
};
//=============================================================================
typedef ListLayout<0> HorizontalListLayout;
typedef ListLayout<1> VerticalListLayout;
//=============================================================================
}
}

#endif
