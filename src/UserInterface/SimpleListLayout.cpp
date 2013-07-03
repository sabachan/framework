#include "stdafx.h"

#include "SimpleListLayout.h"

#include "Magnifier.h"

namespace sg {
namespace ui {
//=============================================================================
template <size_t direction>
SimpleListLayout<direction>::SimpleListLayout(ui::Magnifier const& iMagnifier)
    : m_magnifier(&iMagnifier)
    , m_items()
    , m_subContainer(new SubContainer)
    , m_properties()
    , m_offset(0,0)
#if SG_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD
    , m_lastBiggestChild()
#endif
{
    RequestAddToBack(m_subContainer.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
SimpleListLayout<direction>::SimpleListLayout(ui::Magnifier const& iMagnifier, Properties const& iProperties)
    : SimpleListLayout(iMagnifier)
{
    m_properties = iProperties;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
SimpleListLayout<direction>::~SimpleListLayout()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void SimpleListLayout<direction>::VirtualResetOffset()
{
    m_offset.x() = 0;
    m_offset.y() = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void SimpleListLayout<direction>::VirtualAddOffset(float2 const& iOffset)
{
    m_offset.x() += iOffset.x();
    m_offset.y() += iOffset.y();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void SimpleListLayout<direction>::AppendItem(IMovable* iItem)
{
    SG_ASSERT(nullptr != iItem);
    iItem->ResetOffset();
    ui::Component* c = iItem->AsComponent();
    SG_ASSERT(nullptr != c);
#if SG_ENABLE_ASSERT
    refptr<ui::Component> stayAlive = c;
    CheckConstraintsOnItem(iItem);
#endif
    m_items.push_back(iItem);
    m_subContainer->RequestAddToBack(c);
}
//void InsertItemAt(IMovable* iItem, size_t iPos);
//void InsertItemAfter(IMovable* iItem, IMovable* iAfterWhichh);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void SimpleListLayout<direction>::RemoveItem(IMovable* iItem)
{
#if SG_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD
    m_lastBiggestChild = nullptr;
#endif
    SG_ASSERT(iItem->AsComponent()->Parent() == m_subContainer);
    SG_CODE_FOR_ASSERT(bool wasInList = false;)
    for(auto it = m_items.begin(), end = m_items.end(); it != end; ++it)
    {
        if(*it == iItem)
        {
            SG_CODE_FOR_ASSERT(wasInList = true;)
            m_items.erase(it);
            break;
        }
    }
    SG_ASSERT(wasInList);
    m_subContainer->RequestRemove(iItem->AsComponent());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void SimpleListLayout<direction>::RemoveAllItems()
{
    m_items.clear();
    m_subContainer->RequestRemoveAll();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//template <typename T>
//void SortItems(T iComp);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void SimpleListLayout<direction>::VirtualUpdatePlacement()
{
    float const magnification = m_magnifier->Magnification();
    box2f const& parentBox = Parent()->PlacementBox();
    float2 const parentDelta = parentBox.Delta();
    box2f const margin = box2f::FromMinMax(float2(
        -m_properties.margins.left.  Resolve(magnification, parentDelta.x()),
        -m_properties.margins.top.   Resolve(magnification, parentDelta.y())), float2(
         m_properties.margins.right. Resolve(magnification, parentDelta.x()),
         m_properties.margins.bottom.Resolve(magnification, parentDelta.y())));
    float const parentLength = parentDelta[direction];
    float const interItemMargin = m_properties.margins.interItem.Resolve(magnification, parentLength);

    float const widthMargin = margin.Delta()[widthDirection];
    float const parentInducedWidth = parentDelta[widthDirection] - widthMargin;
    float const maxChildrenWidth = [&]()
    {
        if(FitMode::FitToFrameOnly == m_properties.widthFitMode)
            return 0.f;
        float maxChildrenWidth = 0;
        box2f dummyBox = box2f::FromMinMax(float2(0,0), float2(0,0));
        if(FitMode::FitToMaxContentAndFrame == m_properties.widthFitMode)
            dummyBox.max._[widthDirection] = parentInducedWidth;
        this->SetPlacementBox(dummyBox);
        m_subContainer->SetPlacementBox(dummyBox);
#if SG_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD
        if(nullptr != m_lastBiggestChild)
        {
            SG_ASSERT(m_lastBiggestChild->AsComponent()->Parent() == m_subContainer);
            ui::Component* c = m_lastBiggestChild->AsComponent();
            box2f const& b = c->PlacementBox();
            float2 const delta = b.Delta();
            maxChildrenWidth = std::max(maxChildrenWidth, delta[widthDirection]);
            if(dummyBox.max._[widthDirection] < maxChildrenWidth)
            {
                dummyBox.max._[widthDirection] = maxChildrenWidth;
                this->SetPlacementBox(dummyBox);
                m_subContainer->SetPlacementBox(dummyBox);
            }
        }
#endif
        for(auto const& it : m_items)
        {
            ui::Component* c = it->AsComponent();
            box2f const& b = c->PlacementBox();
            float2 const delta = b.Delta();
#if SG_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD
            if(maxChildrenWidth < delta[widthDirection])
            {
                maxChildrenWidth = delta[widthDirection];
                m_lastBiggestChild = it;
                dummyBox.max._[widthDirection] = maxChildrenWidth;
                this->SetPlacementBox(dummyBox);
                m_subContainer->SetPlacementBox(dummyBox);
            }
#else
            maxChildrenWidth = std::max(maxChildrenWidth, delta[widthDirection]);
#endif
        }
        return maxChildrenWidth;
    } ();
    float const subWidth = [&]()
    {
        switch(m_properties.widthFitMode)
        {
        case FitMode::FitToFrameOnly: return parentInducedWidth; break;
        case FitMode::FitToContentOnly: return maxChildrenWidth; break;
        case FitMode::FitToMaxContentAndFrame: return std::max(parentInducedWidth, maxChildrenWidth); break;
        case FitMode::FitToMinContentAndFrame: return std::min(parentInducedWidth, maxChildrenWidth); break;
        default: SG_ASSUME_NOT_REACHED(); return 0.f;
        }
    } ();
    float2 subSize(uninitialized);
    subSize[direction] = 0;
    subSize[widthDirection] = subWidth;

    float const sumChildrenLength = [&]()
    {
        float sumChildrenLength = 0;
        SG_CODE_FOR_ASSERT(float maxChildrenWidthForAssert = 0;)
        box2f const widthBox = box2f::FromMinMax(float2(0,0), subSize);
        SetPlacementBox(box2f::FromMinMax(float2(0), float2(0)));
        m_subContainer->SetPlacementBox(widthBox);
        for(auto const& it : m_items)
        {
            ui::Component* c = it->AsComponent();
            box2f const& b = c->PlacementBox();
            float prevOffset = b.Min()[direction];
            float2 offsetToAdd = float2(0); offsetToAdd[direction] = sumChildrenLength - prevOffset;
            it->AddOffset(offsetToAdd);
            float2 const delta = b.Delta();
            SG_CODE_FOR_ASSERT(maxChildrenWidthForAssert = std::max(maxChildrenWidth, delta[1-direction]);)
            SG_ASSERT(m_properties.widthFitMode == FitMode::FitToFrameOnly || m_properties.widthFitMode == FitMode::FitToMaxContentAndFrame || delta[1-direction] <= maxChildrenWidth);
            sumChildrenLength += delta[direction];
            sumChildrenLength += interItemMargin;
        }
        SG_ASSERT(m_properties.widthFitMode == FitMode::FitToFrameOnly || maxChildrenWidthForAssert >= maxChildrenWidth);
        sumChildrenLength -= interItemMargin;
        return sumChildrenLength;
    } ();

    subSize[direction] = sumChildrenLength;

    box2f const box = box2f::FromMinDelta(parentBox.Min() + m_offset, subSize + margin.Delta());
    box2f const subBox = box2f::FromMinDelta(parentBox.Min() + m_offset - margin.Min(), subSize);

    SetPlacementBox(box);
    m_subContainer->SetPlacementBox(subBox);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
template <size_t direction>
void SimpleListLayout<direction>::CheckConstraintsOnItem(IMovable* iItem)
{
    // If FitMode == FitToFrame, there is no constraint on item.
    // Else, at a given date, an Item must verify following constraint:
    // - There exists a width W (the minimum width of the item) such that,
    //   if the parent width is less than W, the item width is W and, if
    //   the parent width is more than W, the item width is greater or
    //   equal to W and less or equal to its parent width.
    // - After offset has been reset, If parent width is greater than the
    //   item minimum width, then the item is fully contained by the parent
    //   along width direction.
    // Note that, as these constraints must be verified "at a given date",
    // the width of the item can depends on the date or any other parameter
    // that does not depend on UI placement.

    ui::Component* c = iItem->AsComponent();
    SG_ASSERT(c->IsRefCounted_ForAssert());
    // In the case the iTem is transfered from one parent to another, it
    // may be impossible to check it, as it cannot be added to the test
    // container.
    if(nullptr != c->Parent())
        return;

    ui::RootContainer testContainer;
    testContainer.RequestAddToFront(c);
    SG_ASSERT(c->Parent() == &testContainer);
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(0)));
    box2f const box0 = c->PlacementBox();
    float const widthAt0 = box0.Delta()[widthDirection];
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(0.5f * widthAt0)));
    box2f const box1 = c->PlacementBox();
    SG_ASSERT_MSG(widthAt0 == box1.Delta()[widthDirection],
                "The minimum width of the item must be constant.");
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(widthAt0)));
    box2f const box2 = c->PlacementBox();
    SG_ASSERT_MSG(widthAt0 == box2.Delta()[widthDirection],
                "The minimum width of the item must be constant.");
    SG_ASSERT_MSG(0 == box2.Min()[widthDirection],
                "The item must be fully contained by the parent along width direction when parent is greater than minimum width.");
    SG_ASSERT_MSG(widthAt0 == box2.Max()[widthDirection],
                "The item must be fully contained by the parent along width direction when parent is greater than minimum width.");

    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(2 * widthAt0)));
    box2f const box3 = c->PlacementBox();
    SG_ASSERT_MSG(widthAt0 <= box3.Delta()[widthDirection],
                "The width of the item must be greater than its minimum width (defined when its parent has width 0).");
    SG_ASSERT_MSG(2*widthAt0 >= box3.Delta()[widthDirection],
                "The width of the item must be less than its parent width when this width is greater than the item minimum width.");
    SG_ASSERT_MSG(0 <= box3.Min()[widthDirection],
                "The item must be fully contained by the parent along width direction when parent is greater than minimum width.");
    SG_ASSERT_MSG(2*widthAt0 >= box3.Max()[widthDirection],
                "The item must be fully contained by the parent along width direction when parent is greater than minimum width.");

    testContainer.RequestRemove(c);
    SG_ASSERT(nullptr == c->Parent());
}
#endif
//=============================================================================
template SimpleListLayout<0>;
template SimpleListLayout<1>;
//=============================================================================
}
}
