#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "TreeView.h"

#include "Common.h"
#include "Label.h"
#include <Core/StringFormat.h>
#include <Core/StringUtils.h>
#include <Core/Tool.h>
#include <Image/FontSymbols.h>
#include <UserInterface/AnimFactor.h>

namespace sg {
namespace toolsui {
//=============================================================================
namespace {
    wchar_t closeStr[] = { '#','s','y','m','b','o','l','s','{', image::symbols::RightArrow, '}','\0' };
    wchar_t openStr[] = { '#','s','y','m','b','o','l','s','{', image::symbols::DownArrow, '}','\0' };
}
//=============================================================================
Foldable::Foldable()
    : Foldable(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToContentOnly))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Foldable::Foldable(ui::FitMode2 iFitMode)
    : m_frameProperty()
    , m_sensitiveArea()
    , m_boxArea()
    , m_hlist()
    , m_nonFoldableContent()
    , m_foldIcon()
    , m_foldableContent()
    , m_fitMode(iFitMode)
    , m_framing(FrameHeaderOnly)
    , m_isUnfolded(false)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();

    ui::Length const tab = styleGuide.GetLength(common.TreeTab);
    ui::Length2 const margins = styleGuide.GetVector(common.LineMargin1);

    ui::HorizontalListLayout::Properties hlistprop;
    hlistprop.margins.left = margins.x();
    hlistprop.margins.right = margins.x();
    hlistprop.margins.top = margins.y();
    hlistprop.margins.bottom = margins.y();
    hlistprop.margins.interItem; // = 0;
    hlistprop.widthFitMode = m_fitMode.y();
    m_hlist = new ui::HorizontalListLayout(common.GetMagnifier(), hlistprop);
    RequestAddToBack(m_hlist.get());

    m_frameProperty.size = ui::Relative(float2(1, 1));

    m_foldIcon = new Label(closeStr, ui::Unit(-1.f));
    m_hlist->AppendItem(m_foldIcon.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Foldable::~Foldable()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::Unfold(bool iUnfold)
{
    if(iUnfold != m_isUnfolded)
    {
        if(m_isUnfolded && nullptr != m_foldableContent->AsComponent())
            RequestRemove(m_foldableContent->AsComponent());
        m_isUnfolded = iUnfold;
        if(m_isUnfolded && nullptr != m_foldableContent->AsComponent())
            RequestAddToFront(m_foldableContent->AsComponent());
        m_foldIcon->SetText(m_isUnfolded ? openStr : closeStr);
        NotifyObservers();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::SetNonFoldableContent(ui::IMovable* iContent)
{
    if(iContent == m_nonFoldableContent.get())
        return;
    m_hlist->RemoveAllItems();
    m_hlist->AppendItem(m_foldIcon.get());
    if(nullptr != iContent)
    {
        iContent->ResetOffset();
        m_hlist->AppendExpansibleItem(iContent->AsComponent(), ui::Unit(10), 1);
    }
    m_nonFoldableContent = iContent;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::SetFoldableContent(ui::IMovable* iContent)
{
    if(iContent == m_foldableContent)
        return;
    bool wasUnfolded = IsUnfolded();
    Unfold(false);
    m_foldableContent = iContent;
    Unfold(wasUnfolded);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::VirtualResetOffset()
{
    m_frameProperty.offset = ui::Unit(float2(0));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::VirtualAddOffset(float2 const& iOffset)
{
    m_frameProperty.offset += ui::Unit(iOffset);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent)
{
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
    m_sensitiveArea.OnPointerEvent(iContext, iPointerEvent, m_boxArea, *this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    if(0 == iButton)
    {
        RequestFocusIFN();
        MoveToFrontOfAllUI();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    if(0 == iButton)
    {
        FlipFolding();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::VirtualOnFocusableEvent(ui::FocusableEvent const& iFocusableEvent)
{
    focusable_parent_type::VirtualOnFocusableEvent(iFocusableEvent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Foldable::VirtualMoveFocusReturnHasMoved(ui::FocusDirection iDirection)
{
    if(!HasFocus())
    {
        if(IsUnfolded())
        {
            switch(iDirection)
            {
            case ui::FocusDirection::Previous:
            case ui::FocusDirection::Up:
            {
                ui::IFocusable* focusable = m_foldableContent->AsComponent()->FindFocusableIFP();
                if(nullptr != focusable)
                {
                    bool const hasMoved = focusable->RequestMoveFocusReturnHasMoved(iDirection);
                    if(hasMoved)
                        return true;
                }
            }
            }
        }
        RequestFocusIFN();
        return true;
    }
    bool const hasTerminalFocus = HasTerminalFocus();
    bool const hasMoved = focusable_parent_type::VirtualMoveFocusReturnHasMoved(iDirection);
    if(hasMoved)
        return true;

    switch(iDirection)
    {
    case ui::FocusDirection::Left:
    {
        if(IsUnfolded())
        {
            Unfold(false);
            return true;
        }
        break;
    }
    case ui::FocusDirection::Right:
    {
        if(!IsUnfolded())
        {
            Unfold(true);
            return true;
        }
        else if(IsUnfolded() && HasTerminalFocus())
        {
            IFocusable* focusable = m_foldableContent->AsComponent()->FindFocusableIFP();
            if(nullptr != focusable)
            {
                bool hasMoved = focusable->RequestMoveFocusReturnHasMoved(iDirection);
                if(hasMoved)
                    return true;
            }
        }
        break;
    }
    case ui::FocusDirection::Activate:
    {
        FlipFolding();
        return true;
    }
    case ui::FocusDirection::Down:
    case ui::FocusDirection::Next:
    {
        if(IsUnfolded() && HasTerminalFocus())
        {
            IFocusable* focusable = m_foldableContent->AsComponent()->FindFocusableIFP();
            if(nullptr != focusable)
            {
                bool hasMoved = focusable->RequestMoveFocusReturnHasMoved(iDirection);
                if(hasMoved)
                    return true;
            }
        }
        break;
    }
    case ui::FocusDirection::Up:
    case ui::FocusDirection::Previous:
    {
        if(!HasTerminalFocus())
        {
            SG_ASSERT(IsUnfolded());
            RequestTerminalFocusIFN();
            return true;
        }
        break;
    }
    }
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::VirtualOnDraw(ui::DrawContext const& iContext)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
    float const magnification = common.GetMagnifier().Magnification();

    bool drawFrame = false;
    bool frameHeaderOnly = false;
    switch(m_framing)
    {
    case NoFrame:
        drawFrame = false;
        frameHeaderOnly = true;
        break;
    case FrameHeaderOnly:
        drawFrame = true;
        frameHeaderOnly = true;
        break;
    case FrameAll:
        drawFrame = true;
        frameHeaderOnly = false;
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }

    if(drawFrame)
    {
        box2f const& frameBox = frameHeaderOnly ? m_hlist->PlacementBox() : PlacementBox();
        ButtonLikeRenderParam renderParam;
        GetButtonLikeRenderParam(renderParam, frameBox, true, IsHover(), IsClicked(), HasTerminalFocus());

        float2 const outDelta = renderParam.outBox.Delta();
        float2 const inDelta = renderParam.inBox.Delta();
        if(AllGreaterStrict(inDelta, float2(0.f)))
            drawer->DrawQuad(iContext, renderParam.inBox, renderParam.fillColor);
        if(outDelta != inDelta)
        {
            if(AllGreaterStrict(inDelta, float2(0.f)))
                drawer->DrawFrame(iContext, renderParam.inBox, renderParam.outBox, renderParam.lineColor);
            else
                drawer->DrawQuad(iContext, renderParam.outBox, renderParam.fillColor);
        }
    }
    else
    {
        box2f const& iconBox = m_foldIcon->PlacementBox();
        ButtonLikeRenderParam renderParam;
        GetButtonLikeRenderParam(renderParam, iconBox, true, IsHover(), IsClicked(), HasTerminalFocus());
        float2 const outDelta = renderParam.outBox.Delta();
        float2 const inDelta = renderParam.inBox.Delta();
        if(IsHover() || IsClicked() || HasTerminalFocus())
        {
            if(AllGreaterStrict(inDelta, float2(0.f)))
                drawer->DrawFrame(iContext, renderParam.inBox, renderParam.outBox, renderParam.lineColor);
            else
                drawer->DrawQuad(iContext, renderParam.outBox, renderParam.fillColor);
        }
    }
    parent_type::VirtualOnDraw(iContext);

}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Foldable::VirtualUpdatePlacement()
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& parentBox = Parent()->PlacementBox();

    float2 contentSize = float2(0);

    for_range(size_t, i, 0, 1)
    {
        box2f const preFrame = m_frameProperty.Resolve(magnification, parentBox, contentSize, m_fitMode);
        SetPlacementBox(preFrame);
        box2f const nonFoldableBox = m_hlist->PlacementBox();

        if(IsUnfolded())
        {
            m_foldableContent->ResetOffset();
            SG_ASSERT(m_foldableContent->AsComponent()->Parent() == this);
            box2f const foldableBox = m_foldableContent->AsComponent()->PlacementBox();
            const ui::VerticalListLayout::Properties prop = common.VerticalListProperties();
            float2 const offset = float2(0.f, nonFoldableBox.Delta().y() + prop.margins.interItem.Resolve(magnification, 0.f));
            m_foldableContent->SetOffset(offset);
            const box2f offsetedFoldableBox = foldableBox + offset;
            box2f contentBox = nonFoldableBox;
            contentBox.Grow(offsetedFoldableBox);
            contentSize = contentBox.Delta();
        }
        else
        {
            contentSize = nonFoldableBox.Delta();
        }
    }

    box2f const frame = m_frameProperty.Resolve(magnification, parentBox, contentSize, m_fitMode);
    SetPlacementBox(frame);
    box2f const nonFoldableBox = m_hlist->PlacementBox();
    m_boxArea.SetBox(nonFoldableBox);
}
//=============================================================================
FoldableList::FoldableList()
    : FoldableList(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToContentOnly))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FoldableList::FoldableList(ui::FitMode2 iFitMode)
    : Foldable(iFitMode)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();

    ui::Length const tab = styleGuide.GetLength(common.TreeTab);
    ui::Length2 const margins = styleGuide.GetVector(common.LineMargin1);

    ui::VerticalListLayout::Properties listprop = common.VerticalListProperties();
    listprop.margins.left = tab;
    listprop.margins.right = ui::Unit(0);
    listprop.margins.top = ui::Unit(0);
    listprop.margins.bottom = ui::Unit(0);
    listprop.margins.interItem;
    listprop.widthFitMode = iFitMode.x();
    m_list = new ui::VerticalListLayout(common.GetMagnifier(), listprop);
    Foldable::SetFoldableContent(m_list.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FoldableList::~FoldableList()
{
}
//=============================================================================
TreeView::TreeView()
    : ui::VerticalListLayout(Common::Get().GetMagnifier(), Common::Get().VerticalListProperties())
    , m_needRefresh(false)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TreeView::~TreeView()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::ExplicitPath(ArrayList<std::string>& oPath, std::string const& iPath)
{
    char const* b = iPath.data();
    do {
        char const* e = strpbrk(b, "/");
        if(nullptr == e)
        {
            oPath.EmplaceBack(b);
        }
        else if('/' == *e)
        {
            oPath.EmplaceBack(b, e - b);
            e += 1;
        }
        else
        {
            SG_ASSERT_NOT_REACHED();
        }
        b = e;
    } while(nullptr != b);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::Insert(std::string const& iPath, ui::IMovable* iContent)
{
    ArrayList<std::string> strPath;
    ExplicitPath(strPath, iPath);
    Insert(strPath.View(), iContent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::Insert(ArrayView<std::string const> iPath, ui::IMovable* iContent)
{
    bool const mustRefresh = InsertReturnMustRefreshChildren(m_children, iPath, iContent);
    if(mustRefresh)
        RequestRefresh();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TreeView::InsertReturnMustRefreshChildren(ArrayList<size_t>& ioChildren, ArrayView<std::string const> iPath, ui::IMovable* iContent)
{
    std::string const& path0 = iPath[0];
    for(size_t i : ioChildren)
    {
        Node& n = m_nodes[i];
        if(n.name == path0)
        {
            if(iPath.Size() == 1)
            {
                SG_ASSERT_MSG(nullptr == n.content, "There's already something in this path");
                n.content = iContent;
                RequestRefreshNode(i);
            }
            else
            {
                bool mustRefresh = InsertReturnMustRefreshChildren(n.children, iPath.Tail(1), iContent);
                if(mustRefresh)
                    RequestRefreshNode(i);
            }
            return false;
        }
    }
    size_t newNodeIndex;
    if(all_ones == m_freeNodeIndex)
    {
        newNodeIndex = m_nodes.size();
        // Here, we have to emplace on ioChildren before touching m_nodes as the
        // insertion of a new node may invalidate the input ArrayList.
        // This is pragmatic decision to take in argument a list, even if it can be
        // invalidated, in order to share this function for nodes and root.
        ioChildren.EmplaceBack(newNodeIndex);
        &m_nodes.EmplaceBack();
    }
    else
    {
        size_t const newNodeIndex = m_freeNodeIndex;
        m_freeNodeIndex = m_nodes[newNodeIndex].nextFreeNodeIndex;
    }
    Node& n = m_nodes[newNodeIndex];
    n.name = iPath[0];
    SG_ASSERT(!n.needRefresh);
    RequestRefreshNode(newNodeIndex);
    if(iPath.Size() == 1)
    {
        n.content = iContent;
    }
    else
    {
        bool mustRefresh = InsertReturnMustRefreshChildren(n.children, iPath.Tail(1), iContent);
        SG_UNUSED(mustRefresh);
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ui::IMovable* TreeView::GetIFP(std::string const& iPath)
{
    ArrayList<std::string> strPath;
    ExplicitPath(strPath, iPath);
    return GetIFP(strPath.View());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ui::IMovable* TreeView::GetIFP(ArrayView<std::string const> iPath)
{
    return GetIFP(m_children, iPath);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ui::IMovable* TreeView::GetIFP(ArrayList<size_t>& ioChildren, ArrayView<std::string const> iPath)
{
    std::string const& path0 = iPath[0];
    for(size_t i : ioChildren)
    {
        Node& n = m_nodes[i];
        if(n.name == path0)
        {
            if(iPath.Size() == 1)
            {
                return n.content.get();
            }
            else
            {
                return GetIFP(n.children, iPath.Tail(1));
            }
        }
    }
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::Remove(std::string const& iPath)
{
    ArrayList<std::string> strPath;
    ExplicitPath(strPath, iPath);
    Remove(strPath.View());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::Remove(ArrayView<std::string const> iPath)
{
    bool const mustRefresh = RemoveReturnMustRefreshChildren(m_children, iPath);
    if(mustRefresh)
        RequestRefresh();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TreeView::RemoveReturnMustRefreshChildren(ArrayList<size_t>& ioChildren, ArrayView<std::string const> iPath)
{
    std::string const& path0 = iPath[0];
    for(size_t const& it : ioChildren)
    {
        size_t const i = it;
        Node& n = m_nodes[i];
        if(n.name == path0)
        {
            if(iPath.Size() == 1)
            {
                SG_ASSERT(nullptr != n.content);
                n.content = nullptr;
                if(n.children.Empty())
                {
                    ioChildren.RemoveAt(&it - ioChildren.data());
                    FreeNode(i);
                    return true;
                }
                else
                    return false;
            }
            else
            {
                bool const mustRefresh = RemoveReturnMustRefreshChildren(n.children, iPath.Tail(1));
                if(mustRefresh)
                {
                    if(n.children.Empty() && nullptr == n.content)
                    {
                        ioChildren.RemoveAt(&it - ioChildren.data());
                        FreeNode(i);
                        return true;
                    }
                    else
                    {
                        RequestRefreshNode(i);
                    }
                }
                return false;
            }
        }
    }
    SG_ASSERT_MSG(false, "path not found");
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::FreeNode(size_t iIndex)
{
    Node& n = m_nodes[iIndex];
    if(n.needRefresh)
    {
        for(auto& it : m_nodesToRefresh)
        {
            if(it == iIndex)
            {
                m_nodesToRefresh.RemoveAt(&it - m_nodesToRefresh.Data());
                break;
            }
        }
        n.needRefresh = false;
    }
    n.nextFreeNodeIndex = m_freeNodeIndex;
    m_freeNodeIndex = iIndex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::RequestRefreshNode(size_t iNodeIndex)
{
    Node& n = m_nodes[iNodeIndex];
    if(!n.needRefresh)
    {
        m_nodesToRefresh.PushBack(iNodeIndex);
        n.needRefresh = true;
        InvalidatePlacement();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::RequestRefresh()
{
    m_needRefresh = true;
    InvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::SortChildren(ArrayList<size_t>& ioChildren)
{
    std::sort(ioChildren.begin(), ioChildren.end(), [&](size_t a, size_t b)
    {
        return m_nodes[a].name < m_nodes[b].name;
    });
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::RefreshNode(Node& iNode)
{
    if(iNode.children.Empty())
    {
        SG_ASSERT(nullptr != iNode.content);
        iNode.foldableList = nullptr;
        iNode.needRefresh = false;
        return;
    }
    if(nullptr == iNode.foldableList)
    {
        if(!iNode.children.Empty())
        {
            iNode.foldableList = new FoldableList();
            if(nullptr != iNode.content)
                iNode.foldableList->SetNonFoldableContent(iNode.content.get());
            else
            {
                if(nullptr == iNode.label)
                    iNode.label = new Label(ConvertUTF8ToUCS2(iNode.name));
                iNode.foldableList->SetNonFoldableContent(iNode.label.get());
            }
        }
    }
    refptr<ui::Component> savedFocus = iNode.foldableList->GetFocusedItemIFP();
    iNode.foldableList->RemoveAllFoldableContent();
    SortChildren(iNode.children);
    for(size_t i : iNode.children)
    {
        Node& n = m_nodes[i];
        if(n.needRefresh)
            RefreshNode(n);
        SG_ASSERT(nullptr != n.foldableList || nullptr != n.content);
        ui::IMovable* m = nullptr != n.foldableList ? n.foldableList.get() : n.content.get();
        iNode.foldableList->AppendFoldableContent(m);
    }
    if(nullptr != savedFocus && savedFocus->IsInGUI())
    {
        ui::IFocusable* focusable = savedFocus->AsFocusableIFP();
        SG_ASSERT(nullptr != focusable);
        focusable->RequestFocusIFN();
    }
    iNode.needRefresh = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::Refresh()
{
    RemoveAllItems();
    SortChildren(m_children);
    for(size_t i : m_children)
    {
        Node& n = m_nodes[i];
        SG_ASSERT(!n.needRefresh);
        SG_ASSERT(nullptr != n.foldableList || nullptr != n.content);
        ui::IMovable* m = nullptr != n.foldableList ? n.foldableList.get() : n.content.get();
        parent_type::AppendItem(m);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TreeView::VirtualUpdatePlacement()
{
    parent_type::VirtualUpdatePlacement();
    for(size_t i : m_nodesToRefresh)
        RefreshNode(m_nodes[i]);
    if(m_needRefresh)
        Refresh();
    if(m_needRefresh || !m_nodesToRefresh.Empty())
        parent_type::VirtualUpdatePlacement();
    m_needRefresh = false;
    m_nodesToRefresh.Clear();
}
//=============================================================================
}
}

#endif
