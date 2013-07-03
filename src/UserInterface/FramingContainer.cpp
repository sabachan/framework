#include "stdafx.h"

#include "FramingContainer.h"

#include "Magnifier.h"

namespace sg {
namespace ui {
//=============================================================================
FramingContainer::FramingContainer(Magnifier const& iMagnifier, FrameProperty const& iFrameProperty, FitMode2 iFitMode)
    : m_magnifier(&iMagnifier)
    , m_content(nullptr)
    , m_frameProperty(iFrameProperty)
    , m_fitMode(iFitMode)
    , m_movableOffset(0)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FramingContainer::~FramingContainer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void FramingContainer::SetContent(Component* iContent)
{
    SG_ASSERT(nullptr == m_content || nullptr == iContent);
    if(nullptr != m_content)
    {
        Component* c = m_content.get();
        m_content = nullptr;
        RequestRemove(c);
    }
    if(nullptr != iContent)
    {
        RequestAddToFront(iContent);
        m_content = iContent;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void FramingContainer::VirtualResetOffset()
{
    m_movableOffset = float2(0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void FramingContainer::VirtualAddOffset(float2 const& iOffset)
{
    m_movableOffset += iOffset;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void FramingContainer::VirtualUpdatePlacement()
{
    float const magnification = m_magnifier->Magnification();
    box2f const& parentBox = Parent()->PlacementBox();
    float2 prevContentSize = float2(0);
    for(;;)
    {
        box2f const frame = m_frameProperty.Resolve(magnification, parentBox, prevContentSize, m_fitMode) + m_movableOffset;
        SetPlacementBox(frame);
        box2f const contentBox = m_content->PlacementBox();
        float2 const contentSize = contentBox.Delta();
        if(EqualsWithTolerance(prevContentSize, contentSize, 0.01f))
        {
            break;
        }
        prevContentSize = contentSize;
    }
}
//=============================================================================
}
}

