#include "stdafx.h"

#include "ListLayout.h"

#include "Magnifier.h"

namespace sg {
namespace ui {
//=============================================================================
template <size_t direction>
ListLayout<direction>::Cell::Cell(Component* iContent, Length iMinLength, float iWeight)
    //: m_magnifier(&iMagnifier)
    : m_content(iContent)
    , m_minLength(iMinLength)
    , m_weight(iWeight)
    , m_resolvedMinLength(-1)
    , m_requestedLength(0)
    , m_offset(0)
{
    SG_ASSERT(nullptr != iContent);
    RequestAddToFront(iContent);
    SG_ASSERT(0 == m_minLength.relative);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void ListLayout<direction>::Cell::VirtualUpdatePlacement()
{
    //float const magnification = m_magnifier->Magnification();
    box2f const& parentBox = Parent()->PlacementBox();
    float2 delta0(uninitialized);
    //float const minLength = m_minLength.Resolve(magnification, std::numeric_limits<float>::max());
    float const minLength = m_resolvedMinLength;
    float const requestedLength = m_requestedLength;
    delta0[direction] = std::max(minLength, requestedLength);
    delta0[widthDirection] = parentBox.Delta()[widthDirection];
    SetPlacementBox(box2f::FromMinDelta(float2(0), delta0));
    box2f const contentBox0 = m_content->PlacementBox();
    float2 delta(uninitialized);
    delta = componentwise::max(delta0, contentBox0.Delta());
    SetPlacementBox(box2f::FromMinDelta(parentBox.Min() + m_offset, delta));
}
//=============================================================================
template <size_t direction>
ListLayout<direction>::ListLayout(ui::Magnifier const& iMagnifier)
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
ListLayout<direction>::ListLayout(ui::Magnifier const& iMagnifier, Properties const& iProperties)
    : ListLayout(iMagnifier)
{
    m_properties = iProperties;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
ListLayout<direction>::~ListLayout()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void ListLayout<direction>::VirtualResetOffset()
{
    m_offset.x() = 0;
    m_offset.y() = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void ListLayout<direction>::VirtualAddOffset(float2 const& iOffset)
{
    m_offset.x() += iOffset.x();
    m_offset.y() += iOffset.y();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void ListLayout<direction>::PrivateAppendItem(IMovable* iAsMovable, Cell* iAsCell)
{
    SG_ASSERT(nullptr != iAsMovable);
    SG_ASSERT(nullptr == iAsCell || iAsMovable == iAsCell);
    iAsMovable->ResetOffset();
    ui::Component* c = iAsMovable->AsComponent();
    SG_ASSERT(nullptr != c);
#if SG_ENABLE_ASSERT
    refptr<ui::Component> stayAlive = c;
    CheckConstraintsOnItem(iAsMovable);
#endif
    m_items.emplace_back(iAsMovable, iAsCell);
    m_subContainer->RequestAddToBack(c);
}
//void InsertItemAt(IMovable* iItem, size_t iPos);
//void InsertItemAfter(IMovable* iItem, IMovable* iAfterWhichh);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void ListLayout<direction>::AppendExpansibleItem(Component* iItem, Length iMinLength, float iWeight)
{
    SG_ASSERT(0 == iMinLength.relative);
    //Cell* cell = new Cell(*m_magnifier, iItem, iMinLength, iWeight);
    Cell* cell = new Cell(iItem, iMinLength, iWeight);
    PrivateAppendItem(cell, cell);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void ListLayout<direction>::RemoveItem(Component* iItem)
{
#if SG_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD
    m_lastBiggestChild = nullptr;
#endif
    SG_CODE_FOR_ASSERT(bool wasInList = false;)
    if(iItem->Parent() == m_subContainer)
    {
        for(auto it = m_items.begin(), end = m_items.end(); it != end; ++it)
        {
            if(it->asMovable->AsComponent() == iItem)
            {
                SG_CODE_FOR_ASSERT(wasInList = true;)
                m_items.erase(it);
                m_subContainer->RequestRemove(iItem);
                break;
            }
        }
    }
    else
    {
        SG_ASSERT(nullptr != iItem->Parent() && iItem->Parent()->Parent() == m_subContainer);
        for(auto it = m_items.begin(), end = m_items.end(); it != end; ++it)
        {
            if(it->asCell->m_content == iItem)
            {
                SG_CODE_FOR_ASSERT(wasInList = true;)
                m_items.erase(it);
                m_subContainer->RequestRemove(it->asCell.get());
                break;
            }
        }
    }
    SG_ASSERT(wasInList);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void ListLayout<direction>::RemoveAllItems()
{
    m_items.clear();
    m_subContainer->RequestRemoveAll();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//template <typename T>
//void SortItems(T iComp);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <size_t direction>
void ListLayout<direction>::VirtualUpdatePlacement()
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

    float const lengthMargin = margin.Delta()[direction];
    float const parentInducedLength = parentDelta[direction] - lengthMargin;
    float const widthMargin = margin.Delta()[widthDirection];
    float const parentInducedWidth = parentDelta[widthDirection] - widthMargin;
    float2 const topLeft = parentBox.Min() + m_offset;
    float2 const subTopLeft = topLeft - margin.Min();

    bool anyCellMinLengthHasChanged = false;
    for(auto const& it : m_items)
    {
        if(nullptr == it.asCell)
            continue;
        float minLength = it.asCell->m_minLength.Resolve(magnification, std::numeric_limits<float>::max());
        if(minLength != it.asCell->m_resolvedMinLength)
            anyCellMinLengthHasChanged = true;
        it.asCell->m_resolvedMinLength = minLength;
    }
    if(anyCellMinLengthHasChanged)
    {
        for(auto const& it : m_items)
        {
            if(nullptr == it.asCell)
                continue;
            it.asCell->m_requestedLength = 0.f;
        }
    }

    this->SetPlacementBox(box2f::FromMinMax(topLeft, float2(0,0)));

    float2 subSize = float2(0);
    bool hasConverged;
    SG_CODE_FOR_ASSERT(size_t iterCount = 0;)
    do
    {
        hasConverged = true;
        float2 const prevSubSize = subSize;
        switch(m_properties.widthFitMode)
        {
        case FitMode::FitToFrameOnly: subSize[widthDirection] = parentInducedWidth; break;
        case FitMode::FitToContentOnly: break;
        case FitMode::FitToMaxContentAndFrame: subSize[widthDirection] = std::max(parentInducedWidth, subSize[widthDirection]); break;
        case FitMode::FitToMinContentAndFrame: subSize[widthDirection] = std::min(parentInducedWidth, subSize[widthDirection]); break;
        default: SG_ASSUME_NOT_REACHED();
        }
        box2f const subBox = box2f::FromMinDelta(subTopLeft, subSize);
        m_subContainer->SetPlacementBox(subBox);

        SG_CODE_FOR_ASSERT(float lengthExpansion = 0;)
        float totalCellWeight = 0;
        float totalCellExtraLength = 0;
        float reducibleCellWeight = 0;
        float reducibleCellExtraLength = 0;
        float maxChildrenWidth = 0;
        float2 offset = float2(0);
        for(auto const& it : m_items)
        {
            SG_ASSERT(nullptr != it.asMovable);
            it.asMovable->SetOffset(offset);
            ui::Component* c = it.asMovable->AsComponent();
            box2f const& b = c->PlacementBox();
            float2 const delta = b.Delta();
            maxChildrenWidth = std::max(maxChildrenWidth, delta[widthDirection]);
            offset._[direction] += delta._[direction];
            offset._[direction] += interItemMargin;
            if(nullptr != it.asCell)
            {
                it.asCell->m_isConstrained = false;
                totalCellWeight += it.asCell->m_weight;
                float const realLength = delta._[direction];
                float const requestedLength = it.asCell->m_requestedLength;
                float const minLength = it.asCell->m_resolvedMinLength;
                float const extraLength = realLength - minLength;
                totalCellExtraLength += extraLength;
                if(requestedLength == realLength && 0.f != requestedLength)
                {
                    reducibleCellWeight += it.asCell->m_weight;
                    reducibleCellExtraLength += extraLength;
                    SG_ASSERT(0 == lengthExpansion || EqualsWithTolerance(requestedLength - minLength, lengthExpansion * it.asCell->m_weight, 0.001f));
                    SG_CODE_FOR_ASSERT(lengthExpansion = (requestedLength - minLength) / it.asCell->m_weight;)
                }
            }
        }

        float const sumChildrenLength = offset._[direction];
        subSize._[direction] = sumChildrenLength;
        if(sumChildrenLength > parentInducedLength && reducibleCellWeight > 0.f)
        {
            hasConverged = false;
            subSize._[widthDirection] = 0;
            if(reducibleCellExtraLength < sumChildrenLength-parentInducedLength)
            {
                for(auto const& it : m_items)
                {
                    if(nullptr == it.asCell)
                        continue;
                    it.asCell->m_requestedLength = 0.f;
                    it.asCell->InvalidatePlacement();
                }
            }
            else
            {
                float const deltaLength = parentInducedLength - sumChildrenLength;
                float const extraLengthToDistribute = deltaLength + reducibleCellExtraLength;
                float const extraLengthPerWeight = extraLengthToDistribute / reducibleCellWeight;
                for(auto const& it : m_items)
                {
                    if(nullptr == it.asCell)
                        continue;
                    box2f const& b = it.asCell->PlacementBox();
                    float2 const delta = b.Delta();
                    float const realLength = delta._[direction];
                    it.asCell->m_requestedLength = extraLengthPerWeight * it.asCell->m_weight + it.asCell->m_resolvedMinLength;
                }
            }
        }
        else if(sumChildrenLength < parentInducedLength && totalCellWeight > 0)
        {
            hasConverged = false;
            subSize._[widthDirection] = 0;
            float const deltaLength = parentInducedLength - sumChildrenLength;
            Cell* moreConstrainedCell;
            SG_CODE_FOR_ASSERT(size_t iterCountExtraLength = 0;)
            do
            {
                float biggestConstraintForce = 0;
                moreConstrainedCell = nullptr;
                float const extraLengthToDistribute = deltaLength + totalCellExtraLength;
                float const extraLengthPerWeight = extraLengthToDistribute / totalCellWeight;
                for(auto const& it : m_items)
                {
                    if(nullptr == it.asCell)
                        continue;
                    if(it.asCell->m_isConstrained)
                        continue;
                    box2f const& b = it.asCell->PlacementBox();
                    float2 const delta = b.Delta();
                    float const realLength = delta._[direction];
                    it.asCell->m_requestedLength = extraLengthPerWeight * it.asCell->m_weight + it.asCell->m_resolvedMinLength;
                    SG_ASSERT(it.asCell->m_requestedLength > it.asCell->m_resolvedMinLength - 0.001f);
                    if(it.asCell->m_requestedLength < realLength)
                    {
                        float const constraintForce = (realLength - it.asCell->m_requestedLength) / it.asCell->m_weight;
                        if(constraintForce > biggestConstraintForce)
                        {
                            biggestConstraintForce = constraintForce;
                            moreConstrainedCell = it.asCell.get();
                        }
                    }
                }
                if(nullptr != moreConstrainedCell)
                {
                    moreConstrainedCell->m_isConstrained = true;
                    box2f const& b = moreConstrainedCell->PlacementBox();
                    float2 const delta = b.Delta();
                    float const realLength = delta._[direction];
                    totalCellWeight -= moreConstrainedCell->m_weight;
                    totalCellExtraLength -= realLength - moreConstrainedCell->m_resolvedMinLength;
                }
                SG_ASSERT(totalCellWeight > 0);
                SG_CODE_FOR_ASSERT(++iterCountExtraLength;)
                SG_ASSERT(iterCountExtraLength < 10);
            } while(nullptr != moreConstrainedCell);
        }

        switch(m_properties.widthFitMode)
        {
        case FitMode::FitToFrameOnly: subSize[widthDirection] = parentInducedWidth; break;
        case FitMode::FitToContentOnly: subSize[widthDirection] = maxChildrenWidth; break;
        case FitMode::FitToMaxContentAndFrame: subSize[widthDirection] = std::max(parentInducedWidth, maxChildrenWidth); break;
        case FitMode::FitToMinContentAndFrame: subSize[widthDirection] = std::min(parentInducedWidth, maxChildrenWidth); break;
        default: SG_ASSUME_NOT_REACHED();
        }

        SG_CODE_FOR_ASSERT(++iterCount;)
        SG_ASSERT(iterCount < 6);
        if(subSize != prevSubSize)
            hasConverged = false;
    } while(!hasConverged);

    box2f const box = box2f::FromMinDelta(topLeft, subSize + margin.Delta());
    SetPlacementBox(box);
    box2f const subBox = box2f::FromMinDelta(subTopLeft, subSize);
    m_subContainer->SetPlacementBox(subBox);

#if 0
    float maxChildrenWidth = [&]
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
        if(-1 != m_lastBiggestChildIndex)
        {
            SG_ASSERT(m_lastBiggestChildIndex < m_items.size());
            Item& item = m_items[m_lastBiggestChildIndex];
            if(nullptr != item.asCell)
            {
                // TODO
            }
            else
            {
                SG_ASSERT(item.asMovable->AsComponent()->Parent() == m_subContainer);
                ui::Component* c = item.asMovable->AsComponent();
                box2f const& b = c->PlacementBox();
                float2 const delta = b.Delta();
                maxChildrenWidth = std::max(maxChildrenWidth, delta[widthDirection]);
            }
            if(dummyBox.max._[widthDirection] < maxChildrenWidth)
            {
                dummyBox.max._[widthDirection] = maxChildrenWidth;
                this->SetPlacementBox(dummyBox);
                m_subContainer->SetPlacementBox(dummyBox);
            }
        }
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
            SG_ASSERT(nullptr != it.asMovable);
            ui::Component* c;
            if(nullptr != it.asCell)
            {
                // In that case, as the width may depend on the length, we will
                // take the width at maximum length (length of parent),
                // assuming it is the minimum possible width of this item.
                // If this is not the case, rendering may be broken.
                box2f dummyBoxMaximumLength = dummyBox;
                SG_ASSERT(0 == dummyBoxMaximumLength.min[direction]);
                dummyBoxMaximumLength.max[direction] = parentInducedLength;
                it.asCell->SetPlacementBox(dummyBoxMaximumLength);
                c = it.asCell->m_content.get();
            }
            else
            {
                c = it.asMovable->AsComponent();
            }
            box2f const& b = c->PlacementBox();
            float2 const delta = b.Delta();
#if SG_LIST_LAYOUT_OPTIMISATION_BIGGEST_CHILD
            if(maxChildrenWidth < delta[widthDirection])
            {
                maxChildrenWidth = delta[widthDirection];
                //m_lastBiggestChild = it.asMovable;
                m_lastBiggestChildIndex = &it - &m_items.front();
                dummyBox.max._[widthDirection] = maxChildrenWidth;
                this->SetPlacementBox(dummyBox);
                m_subContainer->SetPlacementBox(dummyBox);
            }
#else
            maxChildrenWidth = std::max(maxChildrenWidth, delta[widthDirection]);
#endif
            if(nullptr != it.asCell)
                it.asCell->InvalidatePlacement();
        }
        return maxChildrenWidth;
    } ();

    float2 subSize(uninitialized);

    auto ComputeSubWidth = [&]()
    {
        switch(m_properties.widthFitMode)
        {
        case FitMode::FitToFrameOnly: return parentInducedWidth; break;
        case FitMode::FitToContentOnly: return maxChildrenWidth; break;
        case FitMode::FitToMaxContentAndFrame: return std::max(parentInducedWidth, maxChildrenWidth); break;
        case FitMode::FitToMinContentAndFrame: return std::min(parentInducedWidth, maxChildrenWidth); break;
        default: SG_ASSUME_NOT_REACHED(); return 0.f;
        }
    };

    subSize[direction] = 0;
    subSize[widthDirection] = ComputeSubWidth();

    bool widthHasChanged = false;
    size_t loop = 0;
    do
    {
    float sumChildrenLength = 0;
    {
        SG_CODE_FOR_ASSERT(float maxChildrenWidthForAssert = 0;)
        box2f const widthBox = box2f::FromMinMax(float2(0,0), subSize);
        this->SetPlacementBox(box2f::FromMinMax(float2(0), float2(0)));
        m_subContainer->SetPlacementBox(widthBox);
        for(auto const& it : m_items)
        {
            if(nullptr != it.asCell)
            {
                it.asCell->ResetExpandedLength();
            }
            SG_ASSERT(nullptr != it.asMovable);
            ui::Component* c = it.asMovable->AsComponent();
            box2f const& b = c->PlacementBox();
            float prevOffset = b.Min()[direction];
            float2 offsetToAdd = float2(0); offsetToAdd[direction] = sumChildrenLength - prevOffset;
            it.asMovable->AddOffset(offsetToAdd);
            float2 const delta = b.Delta();
            SG_CODE_FOR_ASSERT(maxChildrenWidthForAssert = std::max(maxChildrenWidth, delta[1-direction]);)
            SG_ASSERT(m_properties.widthFitMode == FitMode::FitToFrameOnly || m_properties.widthFitMode == FitMode::FitToMaxContentAndFrame || delta[widthDirection] <= maxChildrenWidth || nullptr != it.asCell);
            sumChildrenLength += delta[direction];
            sumChildrenLength += interItemMargin;
        }
        SG_ASSERT(m_properties.widthFitMode == FitMode::FitToFrameOnly || maxChildrenWidthForAssert >= maxChildrenWidth);
        sumChildrenLength -= interItemMargin;
    }

    subSize[direction] = sumChildrenLength;

    if(sumChildrenLength < parentInducedLength)
    {
        float sumExtensibleWeight = 0;
        for(auto const& it : m_items)
        {
            if(nullptr != it.asCell)
                sumExtensibleWeight = it.asCell->m_weight;
        }

        if(0 != sumExtensibleWeight)
        {
            float maxChildrenWidthAfterExtension = maxChildrenWidth;
            float const lengthToAdd = parentInducedLength - sumChildrenLength;
            float const weightedLengthToAdd = lengthToAdd / sumExtensibleWeight;
            float2 offsetToAdd = float2(0);
            for(auto const& it : m_items)
            {
                SG_ASSERT(nullptr != it.asMovable);
                it.asMovable->AddOffset(offsetToAdd);
                if(nullptr != it.asCell)
                {
                    float const weight = it.asCell->m_weight;
                    float const toAdd = weight * weightedLengthToAdd;
                    box2f const& box = it.asCell->PlacementBox();
                    float const length = box.Delta()[direction];
                    it.asCell->SetExpandedLength(length + toAdd);
                    offsetToAdd[direction] += toAdd;
                }
                box2f const& box = it.asMovable->AsComponent()->PlacementBox();
                maxChildrenWidthAfterExtension = std::max(maxChildrenWidthAfterExtension, box.Delta()[widthDirection]);
            }
            subSize[direction] = parentInducedLength;

            maxChildrenWidth = maxChildrenWidthAfterExtension;
            float const subWidth = ComputeSubWidth();
            widthHasChanged = subWidth != subSize[widthDirection];
            subSize[widthDirection] = subWidth;
        }
    }
    ++loop;
    } while(widthHasChanged);

    box2f const box = box2f::FromMinDelta(parentBox.Min() + m_offset, subSize + margin.Delta());
    box2f const subBox = box2f::FromMinDelta(parentBox.Min() + m_offset - margin.Min(), subSize);

    SetPlacementBox(box);
    m_subContainer->SetPlacementBox(subBox);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
template <size_t direction>
void ListLayout<direction>::CheckConstraintsOnItem(IMovable* iItem)
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
template ListLayout<0>;
template ListLayout<1>;
//=============================================================================
}
}
