#ifndef UserInterface_ImmediateSplitLayout_H
#define UserInterface_ImmediateSplitLayout_H

#include <Math/Box.h>
#include <Math/Vector.h>

namespace sg {
namespace ui {
namespace im {
//=============================================================================
enum class SplitDirection { FromLeft, FromTop, FromRight, FromBottom };
inline int GetAxis(SplitDirection dir) { return (int(dir) & 1); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class SplitLayout
{
public:
    SplitLayout(box2f rect) : m_container(rect), m_remaining(rect) {}
    box2f Split(float2 minWH, SplitDirection dir)
    {
        int const a = GetAxis(dir);
        m_minWH[1-a] = std::max(m_minWH[1-a], minWH[1-a]);
        m_addWH[a] += m_minWH[a];
        m_minWH[a] = 0.f;
        m_addWH[a] += minWH[a];
        box2f availableBox = m_remaining;
        switch(dir)
        {
        case SplitDirection::FromLeft:
            m_remaining.min.x() += minWH.x();
            availableBox.max.x() = m_remaining.min.x();
            break;
        case SplitDirection::FromTop:
            m_remaining.min.y() += minWH.y();
            availableBox.max.y() = m_remaining.min.y();
            break;
        case SplitDirection::FromRight:
            m_remaining.max.x() -= minWH.x();
            availableBox.min.x() = m_remaining.max.x();
            break;
        case SplitDirection::FromBottom:
            m_remaining.max.y() -= minWH.y();
            availableBox.min.y() = m_remaining.max.y();
            break;
        }
        return box2f::FromCenterDelta(availableBox.Center(), componentwise::max(availableBox.Delta(), minWH));
    }
    box2f ContainerRect() const { return m_container; }
    box2f RemainingRect() const { return m_remaining; }
    float2 RequestedMinWH() const { return m_addWH + m_minWH; }
private:
    box2f m_container;
    box2f m_remaining;
    float2 m_addWH;
    float2 m_minWH;
};
//=============================================================================
}
}
}

#endif
