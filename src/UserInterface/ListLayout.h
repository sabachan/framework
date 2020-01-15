#ifndef UserInterface_ListLayout_H
#define UserInterface_ListLayout_H

#include "Container.h"
#include "FitMode.h"
#include "Focusable.h"
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
                 , public IFocusable
{
    static_assert(0 == direction || 1 == direction, "direction must be 0 or 1");
    static size_t const widthDirection = 1 - direction;
    typedef Container parent_type;
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
    //void InsertItemAfter(IMovable* iItem, IMovable* iAfterWhich);
    //void InsertItemBefore(IMovable* iItem, IMovable* iBeforeWhich);
    void RemoveItem(Component* iItem);
    void RemoveAllItems();
    size_t ItemCount() const { return m_items.size(); }
    size_t GetLastFocusIndex() const { return m_lastFocusIndex; }
    Component* GetItem(size_t i) const;
    Component* GetFocusedItemIFP() const;
    //template <typename T>
    //void SortItems(T iComp);
    //ArrayView<safeptr<IMovable> const> Items() const { return ArrayView<safeptr<IMovable> const>(m_items.data(), m_items.size()); }
    Magnifier const* GetMagnifier() { return m_magnifier.get(); }
protected:
    virtual bool VirtualMoveFocusReturnHasMoved(FocusDirection iDirection) override;
    virtual Component* VirtualAsComponent() override { return this; }
    virtual IFocusable* VirtualAsFocusableIFP() override { return this; }
    virtual void VirtualOnInsertInUI() override { parent_type::VirtualOnInsertInUI(); OnInsertFocusableInUI(); }
    virtual void VirtualOnRemoveFromUI() override { OnRemoveFocusableFromUI(); parent_type::VirtualOnRemoveFromUI(); }
    virtual void VirtualOnSetFocusChild(IFocusable* iFocusable) override;
protected:
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
    virtual void VirtualUpdatePlacement() override;
    virtual void VirtualOnChildInvalidatePlacement() override { InvalidatePlacement(); }
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
        virtual ui::Component* VirtualAsComponent() { return this; }
        Component* Content() const { return m_content.get(); }
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
    size_t GetFocusIndex() const;
    bool RequestMoveFocusOnItemReturnHasMoved(FocusDirection iDirection, Item& iItem);
    void PrivateAppendItem(IMovable* iAsMovable, Cell* iAsCell);
#if SG_ENABLE_ASSERT
    void CheckConstraintsOnItem(IMovable* iItem);
#endif
private:
    safeptr<ui::Magnifier const> m_magnifier; // TODO: MagnifiableContainer, MagnifiableComponent
    std::vector<Item> m_items;
    //std::vector<u8> m_isCell;
    safeptr<SubContainer> m_subContainer;
    Properties m_properties;
    float2 m_offset;
    size_t m_lastFocusIndex;
#if SG_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD
    size_t m_lastBiggestChildIndex;
    //safeptr<IMovable> m_lastBiggestChild;
#endif
};
//=============================================================================
typedef ListLayout<0> HorizontalListLayout;
typedef ListLayout<1> VerticalListLayout;
//=============================================================================
//template <size_t direction>
//class FocusableListLayout : public ListLayout
//                          , protected IFocusable
//{
//protected:
//    virtual bool VirtualMoveFocusReturnHasMoved(FocusDirection iDirection) override;
//    virtual ui::Component* VirtualAsComponent() override { return this; }
//    virtual ui::IFocusable* VirtualAsFocusableIFP() override { return this; }
//    virtual void VirtualOnInsertInUI() override { parent_type::VirtualOnInsertInUI(); OnInsertFocusableInUI(); }
//    virtual void VirtualOnRemoveFromUI() override { parent_type::VirtualOnRemoveFromUI(); OnRemoveFocusableFromUI(); }
//private:
//};
////=============================================================================
//typedef FocusableListLayout<0> HorizontalFocusableListLayout;
//typedef FocusableListLayout<1> VerticalFocusableListLayout;
////=============================================================================
}
}

#endif
