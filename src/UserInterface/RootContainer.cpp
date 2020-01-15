#include "stdafx.h"

#include "RootContainer.h"

namespace sg {
namespace ui {
//=============================================================================
RootContainer::RootContainer()
: Container(ComponentIsRoot)
, IFocusable(ComponentIsRoot)
{
    OnInsertInUI();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RootContainer::~RootContainer()
{
    OnRemoveFromUI();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RootContainer::SetPlacementBox(box2f const& iBox)
{
    parent_type::SetPlacementBox(iBox);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void RootContainer::VirtualUpdatePlacement()
{
    SG_ASSERT_NOT_REACHED();
}
//=============================================================================
}
}
