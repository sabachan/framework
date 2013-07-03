#ifndef UserInterface_FitMode_H
#define UserInterface_FitMode_H

#include <Math/Box.h>
#include <Math/Vector.h>

namespace sg {
namespace ui {
//=============================================================================
enum class FitMode
{
    FitToFrameOnly,
    FitToContentOnly,
    FitToMaxContentAndFrame,
    FitToMinContentAndFrame,
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float ComputeFittedSize(FitMode iFitMode, float iFrameSize, float iContentSize)
{
    float size = 0.f;
    switch(iFitMode)
    {
    case ui::FitMode::FitToFrameOnly:           size = iFrameSize; break;
    case ui::FitMode::FitToContentOnly:         size = iContentSize; break;
    case ui::FitMode::FitToMaxContentAndFrame:  size = std::max(iFrameSize, iContentSize); break;
    case ui::FitMode::FitToMinContentAndFrame:  size = std::min(iFrameSize, iContentSize); break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
    return size;
}
//=============================================================================
class FitMode2
{
public:
    FitMode2() : m_x(FitMode::FitToFrameOnly), m_y(FitMode::FitToFrameOnly) {}
    explicit FitMode2(FitMode iBroadcast) : m_x(iBroadcast), m_y(iBroadcast) {}
    explicit FitMode2(FitMode iX, FitMode iY) : m_x(iX), m_y(iY) {}
    FitMode x() const { return m_x; }
    FitMode y() const { return m_y; }
    bool DoesNeedContentSize() const { return FitMode::FitToFrameOnly != m_x || FitMode::FitToFrameOnly != m_y; }
    float2 ComputeTestSize(float2 const& iFrameSize) const
    {
        float2 size = iFrameSize;
        if(FitMode::FitToContentOnly == m_x) size.x() = 0;
        if(FitMode::FitToContentOnly == m_y) size.y() = 0;
        return size;
    }
    box2f ComputeTestBox(box2f const& iFrameBox) const
    {
        SG_ASSERT(DoesNeedContentSize());
        return box2f::FromMinDelta(float2(0), ComputeTestSize(iFrameBox.Delta()));
    }
    float2 ComputeFittedSize(float2 const& iFrameSize, float2 const& iContentSize)
    {
        float2 size(uninitialized);
        size.x() = ::sg::ui::ComputeFittedSize(m_x, iFrameSize.x(), iContentSize.x());
        size.y() = ::sg::ui::ComputeFittedSize(m_y, iFrameSize.y(), iContentSize.y());
        return size;
    }
private:
    FitMode m_x;
    FitMode m_y;
};
//=============================================================================
}
}

#endif
