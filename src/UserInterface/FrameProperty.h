#ifndef UserInterface_FrameProperty_H
#define UserInterface_FrameProperty_H

#include <Math/Box.h>
#include "FitMode.h"
#include "Length.h"

namespace sg {
namespace renderengine {
    class CompositingLayer;
}
}

namespace sg {
namespace ui {
//=============================================================================
// A FrameProperty is a generic powrful way to describe the placement of a box
// relative to another one (the parent). Using Lengths, it can describes its
// size as a composition of a unit size, a magnifiable size and a relative
// size. This enable the description of fixed-size boxes, or boxes whose size
// are relative to their parent, using negative unit or magnifiable length to
// describe external margins. The offset is also usefull mainly to describe
// margins. Also, fine alignment can be done thanks to the anchor, that is
// positioned relatively to parent box, and to which the box is aligned.
struct FrameProperty
{
public:
    Length2 offset;
    Length2 size;
    float2 anchorAlignment;
    float2 alignmentToAnchor;

public:
    box2f Resolve(float iMagnification, box2f const& iParentBox) const
    {
        return ResolveImpl<false, false>(iMagnification, iParentBox);
    }
    box2f Resolve(float iMagnification, box2f const& iParentBox, float2 const& iMinSize) const
    {
        return ResolveImpl<true, false>(iMagnification, iParentBox, iMinSize);
    }
    box2f Resolve(float iMagnification, box2f const& iParentBox, float2 const& iContentSize, FitMode2 iFitMode) const
    {
        return ResolveImpl<false, true>(iMagnification, iParentBox, iContentSize, iFitMode);
    }
private:
    template <bool useMinSize, bool useFitMode>
    box2f ResolveImpl(float iMagnification, box2f const& iParentBox, float2 iMinSize = float2(0), FitMode2 iFitMode = FitMode2(FitMode::FitToMaxContentAndFrame)) const
    {
        static_assert(!useMinSize || !useFitMode, "");
        float2 const parentSize = iParentBox.Delta();
        float2 resolvedSize = size.Resolve(iMagnification, parentSize);
        if(useFitMode)
            resolvedSize = iFitMode.ComputeFittedSize(resolvedSize, iMinSize);
        else if(useMinSize)
            resolvedSize = componentwise::max(resolvedSize, iMinSize);
        float2 const resolvedOffset = offset.Resolve(iMagnification, parentSize);
        float2 const anchor = lerp(iParentBox.min, iParentBox.max, anchorAlignment);
        float2 const minCorner = anchor - resolvedSize * alignmentToAnchor + resolvedOffset;
        box2f const resolvedBox = box2f::FromMinDelta(minCorner, resolvedSize);
        return resolvedBox;
    }
};
//=============================================================================
struct FramePropertyAndFitMode : private FrameProperty
{
public:
    using FrameProperty::offset;
    using FrameProperty::size;
    using FrameProperty::anchorAlignment;
    using FrameProperty::alignmentToAnchor;
    FitMode2 fitMode;
public:
    box2f Resolve(float iMagnification, box2f const& iParentBox, float2 const& iContentSize) const
    {
        return FrameProperty::Resolve(iMagnification, iParentBox, iContentSize, fitMode);
    }
};
//=============================================================================
}
}

#endif
